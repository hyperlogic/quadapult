#ifndef REFBASE_H
#define REFBASE_H

#include "render.h"
#include <map>

class RefBase;

class RefHandle
{
public:
    RefHandle(RefBase* p, int* i, int id) : m_p(p), m_i(i), m_id(id) {}
    RefBase* Get() { return (*m_i == m_id) ? m_p : 0; }
protected:
    RefBase* m_p;
    int* m_i;
    int m_id;
};

class HandleManager
{
public:
    static HandleManager& Get();

    HandleManager();
    void NotifyDelete(RefBase* p);
    RefHandle NewHandle(RefBase* p);
protected:
    static const int kMaxInstances = 16384;
    int m_array[kMaxInstances];
    int m_id;
    std::map<RefBase*, int> m_active;
};

extern HandleManager s_handleManager;

class RefBase
{
public:
    RefBase() : m_refCount(1) {}
    virtual ~RefBase()
    {
        HandleManager::Get().NotifyDelete(this);
    }

    void Ref(int n = 1)
    {
        m_refCount++;
    }

    void UnRef(int n = 1)
    {
        ASSERT(m_refCount > 0);
        m_refCount--;
        if (m_refCount <= 0)
            delete this;
    }

    int m_refCount;
};


#endif
