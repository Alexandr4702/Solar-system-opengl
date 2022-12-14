#include "ShadowMapTech.h"

ShadowMapFBO::ShadowMapFBO()
{
    m_fbo = 0;
    m_shadowMap = 0;
}

ShadowMapFBO::~ShadowMapFBO()
{
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
    }

    if (m_shadowMap != 0) {
        glDeleteTextures(1, &m_shadowMap);
    }
}

bool ShadowMapFBO::Init(unsigned int Width, unsigned int Height, bool ForPCF)
{
    m_width = Width;
    m_height = Height;

    // Create the FBO
    glGenFramebuffers(1, &m_fbo);

    // Create the depth buffer
    glGenTextures(1, &m_shadowMap);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, Width, Height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    GLint FilterType = ForPCF ? GL_LINEAR : GL_NEAREST;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, FilterType);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, FilterType);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);


    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap, 0);

    // Disable writes to the color buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        return false;
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return true;
}


void ShadowMapFBO::BindForWriting()
{
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, m_width, m_height);  // set the width/height of the shadow map!
}


void ShadowMapFBO::BindForReading(GLenum TextureUnit)
{
    glActiveTexture(TextureUnit);
    glBindTexture(GL_TEXTURE_2D, m_shadowMap);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////


CascadedShadowMapFBO::CascadedShadowMapFBO()
{
    m_fbo = 0;
    // ZERO_MEM(m_shadowMap);
}

CascadedShadowMapFBO::~CascadedShadowMapFBO()
{
    if (m_fbo != 0) {
        glDeleteFramebuffers(1, &m_fbo);
    }

    // glDeleteTextures(ARRAY_SIZE_IN_ELEMENTS(m_shadowMap), m_shadowMap);
}

bool CascadedShadowMapFBO::Init(unsigned int WindowWidth, unsigned int WindowHeight)
{
    // Create the FBO
    glGenFramebuffers(1, &m_fbo);

    // Create the depth buffer
    // glGenTextures(ARRAY_SIZE_IN_ELEMENTS(m_shadowMap), m_shadowMap);

    // for (uint i = 0 ; i < ARRAY_SIZE_IN_ELEMENTS(m_shadowMap) ; i++) {
    //     glBindTexture(GL_TEXTURE_2D, m_shadowMap[i]);
    //     glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, WindowWidth, WindowHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    //     glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    // }

    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap[0], 0);

    // Disable writes to the color buffer
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    GLenum Status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

    if (Status != GL_FRAMEBUFFER_COMPLETE) {
        printf("FB error, status: 0x%x\n", Status);
        return false;
    }

    return true;
}


void CascadedShadowMapFBO::BindForWriting(uint CascadeIndex)
{
    // assert(CascadeIndex < ARRAY_SIZE_IN_ELEMENTS(m_shadowMap));
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, m_fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, m_shadowMap[CascadeIndex], 0);
}


void CascadedShadowMapFBO::BindForReading()
{
    // glActiveTexture(CASCACDE_SHADOW_TEXTURE_UNIT0);
    // glBindTexture(GL_TEXTURE_2D, m_shadowMap[0]);

    // glActiveTexture(CASCACDE_SHADOW_TEXTURE_UNIT1);
    // glBindTexture(GL_TEXTURE_2D, m_shadowMap[1]);

    // glActiveTexture(CASCACDE_SHADOW_TEXTURE_UNIT2);
    // glBindTexture(GL_TEXTURE_2D, m_shadowMap[2]);
}
