#include "sprite.h"

Sprite::Sprite()
{

}

void Sprite::SetColor(const Vector4f& color)
{
    m_color = color;
}

const Vector4f& Sprite::GetColor() const
{
    return m_color;
}

Vector4f& Sprite::GetColor()
{
    return m_color;
}

void Sprite::SetPosition(const Vector2f& position)
{
    m_position = position;
}

const Vector2f& Sprite::GetPosition() const
{
    return m_position;
}

Vector2f& Sprite::GetPosition()
{
    return m_position;
}

void Sprite::SetSize(const Vector2f& size)
{
    m_size = size;
}

const Vector2f& Sprite::GetSize() const
{
    return m_size;
}

Vector2f& Sprite::GetSize()
{
    return m_size;
}

void Sprite::SetDepth(float depth)
{
    m_depth = depth;
}

float Sprite::GetDepth() const
{
    return m_depth;
}

void Sprite::Draw() const
{

}
