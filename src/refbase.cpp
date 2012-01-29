#include "refbase.h"

HandleManager s_handleManager;

HandleManager& HandleManager::Get()
{
    return s_handleManager;
}

HandleManager::HandleManager()
{
    memset(m_array, 0, kMaxInstances * sizeof(int));
    m_id = 1;
}

void HandleManager::NotifyDelete(RefBase* p)
{
    std::map<RefBase*, int>::iterator iter = m_active.find(p);
    if (iter != m_active.end())
    {
        int i = iter->second;
        m_active.erase(iter);
        m_array[i] = 0;
    }
}

RefHandle HandleManager::NewHandle(RefBase* p)
{
    std::map<RefBase*, int>::iterator iter = m_active.find(p);
    if (iter != m_active.end())
    {
        int i = iter->second;
        RefHandle h(p, m_array + i, m_array[i]);
        return h;
    }
    else
    {
        int i;
        for (i = 0; i < kMaxInstances; ++i)
        {
            if (m_array[i] == 0)
                break;
        }
        ASSERT(i < kMaxInstances);

        m_array[i] = m_id;
        RefHandle h(p, m_array + i, m_id);
        m_id++;

        m_active[p] = i;

        return h;
    }
}
