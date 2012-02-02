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

    uint8_t colors[16];
    uint8_t r = m_color.x * 255;
    uint8_t g = m_color.y * 255;
    uint8_t b = m_color.z * 255;
    uint8_t a = m_color.w * 255;
    for (int i = 0; i < 4; ++i)
    {
        colors[i * 4 + 0] = r;
        colors[i * 4 + 1] = g;
        colors[i * 4 + 2] = b;
        colors[i * 4 + 3] = a;
    }

    glColorPointer(4, GL_UNSIGNED_BYTE, 0, colors);
    glEnableClientState(GL_COLOR_ARRAY);

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

void Sprite::PushBack(std::vector<float>& vertVec,
                      std::vector<uint8_t>& colorVec,
                      std::vector<float>& uvVec,
                      std::vector<uint16_t>& indexVec) const
{
    int size = vertVec.size() / 2;
    float verts[8];
    verts[0] = m_position.x;
    verts[1] = m_position.y;
    verts[2] = m_position.x + m_size.x;
    verts[3] = m_position.y;
    verts[4] = m_position.x;
    verts[5] = m_position.y + m_size.y;
    verts[6] = m_position.x + m_size.x;
    verts[7] = m_position.y + m_size.y;
    for (int i = 0; i < 8; ++i)
        vertVec.push_back(verts[i]);

    uint8_t r = m_color.x * 255;
    uint8_t g = m_color.y * 255;
    uint8_t b = m_color.z * 255;
    uint8_t a = m_color.w * 255;
    for (int i = 0; i < 4; ++i)
    {
        colorVec.push_back(r);
        colorVec.push_back(g);
        colorVec.push_back(b);
        colorVec.push_back(a);
    }

    static float uvs[] = {0, 0, 1, 0, 0, 1, 1, 1};
    for (int i = 0; i < 8; ++i)
        uvVec.push_back(uvs[i]);

    static uint16_t indices[] = {0, 2, 1, 2, 3, 1};
    for (int i = 0; i < 6; ++i)
        indexVec.push_back(indices[i] + size);
}

void Sprite::DrawVecs(std::vector<float>& vertVec,
                      std::vector<uint8_t>& colorVec,
                      std::vector<float>& uvVec,
                      std::vector<uint16_t>& indexVec)
{
    ASSERT(vertVec.size() % 8 == 0);
    ASSERT(uvVec.size() % 8 == 0);
    ASSERT(colorVec.size() % 16 == 0);
    ASSERT(indexVec.size() % 6 == 0);

    glVertexPointer(2, GL_FLOAT, 0, (void*)&vertVec[0]);
    glEnableClientState(GL_VERTEX_ARRAY);
    
    glColorPointer(4, GL_UNSIGNED_BYTE, 0, (void*)&colorVec[0]);
    glEnableClientState(GL_COLOR_ARRAY);

    glClientActiveTexture(GL_TEXTURE0);
    glTexCoordPointer(2, GL_FLOAT, 0, (void*)&uvVec[0]);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);

    glDrawElements(GL_TRIANGLES, indexVec.size(), GL_UNSIGNED_SHORT, (void*)&indexVec[0]);


/*
    // DUMP!
    printf("DrawVecs()\n");
    printf("  verts =\n");
    for (int i = 0; i < vertVec.size()/2; ++i)
        printf("    %d = (%.5f, %.5f)\n", i, vertVec[i*2], vertVec[i*2 + 1]);
    printf("colors =\n");
    for (int i = 0; i < colorVec.size()/4; ++i)
        printf("    %d = (%d, %d, %d, %d)\n", i, (int)colorVec[i*4], (int)colorVec[i*4+1], (int)colorVec[i*4+2], (int)colorVec[i*4+3]);
    printf("  uvs =\n");
    for (int i = 0; i < uvVec.size()/2; ++i)
        printf("    %d = (%.5f, %.5f)\n", i, uvVec[i*2], uvVec[i*2 + 1]);
    printf("  indices =\n");
    for (int i = 0; i < indexVec.size()/3; ++i)
        printf("    %d = (%d, %d, %d)\n", i, (int)indexVec[i*3], (int)indexVec[i*3+1], (int)indexVec[i*3+2]);
*/

    vertVec.clear();
    colorVec.clear();
    uvVec.clear();
    indexVec.clear();
}

