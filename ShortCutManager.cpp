//
// Created by ZQD on 26-5-18.
//

#include "ShortCutManager.h"

#include <OpenGl_GlCore11Fwd.hxx>
#include <OpenGl_GlCore30.hxx>


ShortCutManager::ShortCutManager(opencascade::handle<V3d_View> view,
                                 opencascade::handle<OpenGl_GraphicDriver> graphicDriver): m_RawView(view),
    m_GraphicDriver(graphicDriver)
{
}

bool ShortCutManager::IsPointVisible(const gp_Pnt& point) const
{
    auto pointOnNDC = m_RawView->Camera()->Project(point);
    int mouseX, mouseY;
    m_RawView->Convert(point.X(), point.Y(), point.Z(), mouseX, mouseY);

    auto glContext = m_GraphicDriver->GetSharedContext();

    std::vector<float> depthTile;
    int height, width;
    m_RawView->Window()->Size(width, height);
    constexpr int TILE_SIZE = 64;
    depthTile.resize(width * height);

    int glY = height - mouseY - 1;
    int startX = mouseX - TILE_SIZE / 2;
    int startY = glY - TILE_SIZE / 2;

    //优化边界
    startX = std::max(0, startX);
    startY = std::max(0, startY);

    if (startX + TILE_SIZE > width)
        startX = width - TILE_SIZE;

    if (startY + TILE_SIZE > height)
        startY = height - TILE_SIZE;
    glContext->core11fwd->glFinish();
    glContext->core11fwd->glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glContext->core11fwd->glReadBuffer(GL_FRONT);
    glContext->core30->glBindFramebuffer(GL_READ_FRAMEBUFFER, 0); // 绑定默认帧缓冲
    glContext->core11fwd->glReadPixels(
        0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depthTile.data());

    GLint fbo = 0;

    glContext->core11fwd->glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &fbo);
    std::cout << fbo << std::endl;

    GLint depthBits = 0;

    glContext->core11fwd->glGetIntegerv(GL_DEPTH_BITS, &depthBits);

    GLint depthType = 0;

    glContext->core30->glGetFramebufferAttachmentParameteriv(
        GL_FRAMEBUFFER,
        GL_DEPTH_ATTACHMENT,
        GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE,
        &depthType);

    std::cout << depthType << std::endl;

    if (false)
    {
        SaveDepthImage(depthTile, width, height);
    }
    if (false)
    {
        std::vector<unsigned char> colorBuffer;
        colorBuffer.resize(width * height * 4);

        glContext->core11fwd->glPixelStorei(
            GL_PACK_ALIGNMENT,
            1);

        glContext->core11fwd->glReadPixels(
            0,
            0,
            width,
            height,
            GL_RGBA,
            GL_UNSIGNED_BYTE,
            colorBuffer.data());

        auto SaveColorImage = [](
            const std::vector<unsigned char>& buffer,
            int width,
            int height)
        {
            std::ofstream out(
                "color.ppm",
                std::ios::binary);

            out << "P6\n";
            out << width << " "
                << height << "\n";
            out << "255\n";

            for (int y = height - 1; y >= 0; --y)
            {
                for (int x = 0; x < width; ++x)
                {
                    int idx =
                        (y * width + x) * 4;

                    out.write(
                        reinterpret_cast<const char*>(
                            &buffer[idx]),
                        3);
                }
            }
        };
        SaveColorImage(colorBuffer, width, height);
    }

    int localX = mouseX - startX;
    int localY = glY - startY;
    float depth = depthTile[localY * TILE_SIZE + localX];
    return pointOnNDC.Z() < depth;
}

void ShortCutManager::SaveDepthImage(const std::vector<float>& depthBuffer, int width, int height)
{
    std::ofstream out(
        R""(C:\Users\ZQD\Desktop\depth.pgm)"",
        std::ios::binary);

    out << "P5\n";
    out << width << " "
        << height << "\n";
    out << "255\n";

    float minDepth = FLT_MAX;
    float maxDepth = -FLT_MAX;

    for (float d : depthBuffer)
    {
        if (d < 1.0f)
        {
            minDepth = std::min(minDepth, d);
            maxDepth = std::max(maxDepth, d);
        }
    }

    std::cout
        << "min=" << minDepth
        << " max=" << maxDepth
        << std::endl;

    for (int y = height - 1; y >= 0; --y)
    {
        for (int x = 0; x < width; ++x)
        {
            float d =
                depthBuffer[y * width + x];

            float nd = 0.0f;

            if (maxDepth > minDepth)
            {
                nd =
                    (d - minDepth)
                    / (maxDepth - minDepth);
            }

            uint8_t gray =
                static_cast<uint8_t>(
                    (1.0f - nd) * 255.0f);

            out.write(
                reinterpret_cast<char*>(&gray),
                1);
        }
    }
}


void ShortCutManager::Test()
{
    bool visible = IsPointVisible(gp_Pnt{0, 0, 0});
}
