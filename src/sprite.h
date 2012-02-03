#ifndef SPRITE_H
#define SPRITE_H

#include "abaci.h"
#include "texture.h"

class Texture;

class Sprite
{
public:
    Sprite();
    ~Sprite();

    void SetColor(const Vector4f& color);
    const Vector4f& GetColor() const;
    Vector4f& GetColor();

    void SetPosition(const Vector2f& position);
    const Vector2f& GetPosition() const;
    Vector2f& GetPosition();

    void SetSize(const Vector2f& size);
    const Vector2f& GetSize() const;
    Vector2f& GetSize();

    void SetDepth(float depth);
    float GetDepth() const;

    void SetTexture(Texture* texture);
	const Texture* GetTexture() const;

    void Draw() const;
	void SetName(const std::string& name) { m_name = name; }
	const std::string& GetName() const { return m_name; }

    void PushBack(std::vector<float>& vertVec,
                  std::vector<uint8_t>& colorVec,
                  std::vector<float>& uvVec,
                  std::vector<uint16_t>& indexVec) const;

    static void DrawVecs(std::vector<float>& vertVec,
                         std::vector<uint8_t>& colorVec,
                         std::vector<float>& uvVec,
                         std::vector<uint16_t>& indexVec);

protected:

    Vector4f m_color;
    Vector2f m_position;
    Vector2f m_size;
    float m_depth;
    Texture* m_texture;
	std::string m_name;
};

#endif
