#include "sprite.h"
#include <stdint.h>
#include "render.h"

Sprite::Sprite() : m_texture(0)
{

}

Sprite::~Sprite()
{
    if (m_texture)
        m_texture->UnRef();
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

void Sprite::SetTexture(Texture* texture)
{
    if (texture)
        texture->Ref();

    if (m_texture)
        m_texture->UnRef();

    m_texture = texture;
}

void Sprite::Draw() const
{
    glColor4f(m_color[0], m_color[1], m_color[2], m_color[3]);

    float verts[8];
    verts[0] = m_position.x;
    verts[1] = m_position.y;
    verts[2] = m_position.x + m_size.x;
    verts[3] = m_position.y;
    verts[4] = m_position.x;
    verts[5] = m_position.y + m_size.y;
    verts[6] = m_position.x + m_size.x;
    verts[7] = m_position.y + m_size.y;

    glVertexPointer(2, GL_FLOAT, 0, verts);
    glEnableClientState(GL_VERTEX_ARRAY);

    static float uvs[] = {0, 0, 1, 0, 0, 1, 1, 1};

    // assume glEnable(GL_TEXTURE2D);
    glClientActiveTexture(GL_TEXTURE0);
    glTexCoordPointer(2, GL_FLOAT, 0, uvs);

    glActiveTexture(GL_TEXTURE0);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glBindTexture(GL_TEXTURE_2D, m_texture->GetTexture());

    static uint16_t indices[] = {0, 2, 1, 2, 3, 1};
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}
