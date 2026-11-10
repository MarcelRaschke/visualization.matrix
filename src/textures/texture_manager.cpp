/*
 *  Copyright (C) 2005-2021 Team Kodi <https://kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 *
 *  DCO: I certify that this contribution is licensed under the GPL-2.0-or-later
 *       and I have the right to submit it under this license.
 */

#include "texture_manager.h"
#include "../main.h"

#include <kodi/Filesystem.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <algorithm>

// Texture RAII implementation
Texture::~Texture()
{
  Release();
}

Texture::Texture(Texture&& other) noexcept : m_textureId(other.m_textureId)
{
  other.m_textureId = 0;
}

Texture& Texture::operator=(Texture&& other) noexcept
{
  if (this != &other)
  {
    Release();
    m_textureId = other.m_textureId;
    other.m_textureId = 0;
  }
  return *this;
}

void Texture::Release()
{
  if (m_textureId != 0)
  {
    glDeleteTextures(1, &m_textureId);
    m_textureId = 0;
  }
}

// TextureManager implementation
TextureManager::~TextureManager()
{
  ClearAll();
  if (m_effectFb != 0)
  {
    glDeleteFramebuffers(1, &m_effectFb);
    m_effectFb = 0;
  }
}

Texture TextureManager::CreateTexture(GLint format, unsigned int w, unsigned int h, const GLvoid* data)
{
  GLuint textureId = 0;
  glActiveTexture(GL_TEXTURE0);
  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, format, w, h, 0, format, GL_UNSIGNED_BYTE, data);
  glBindTexture(GL_TEXTURE_2D, 0);

  return Texture(textureId);
}

Texture TextureManager::CreateTexture(const GLvoid* data,
                                      GLint format,
                                      unsigned int w,
                                      unsigned int h,
                                      GLint internalFormat,
                                      GLint scaling,
                                      GLint repeat)
{
  GLuint textureId = 0;
  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, scaling);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, scaling);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, repeat);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, repeat);

  glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, w, h, 0, format, GL_UNSIGNED_BYTE, data);
  glBindTexture(GL_TEXTURE_2D, 0);

  return Texture(textureId);
}

Texture TextureManager::CreateTexture(const std::string& file,
                                      GLint internalFormat,
                                      GLint scaling,
                                      GLint repeat)
{
  kodi::Log(ADDON_LOG_DEBUG, "creating texture %s\n", file.c_str());

  int width = 0, height = 0, n = 0;
  unsigned char* image = nullptr;
  stbi_set_flip_vertically_on_load(true);

  image = stbi_load(file.c_str(), &width, &height, &n, STBI_rgb_alpha);
  if (image == nullptr)
  {
    kodi::Log(ADDON_LOG_ERROR, "couldn't load image");
    return Texture();
  }

  Texture texture = CreateTexture(image,
                                  GL_RGBA,
                                  static_cast<unsigned int>(width),
                                  static_cast<unsigned int>(height),
                                  internalFormat,
                                  scaling,
                                  repeat);
  stbi_image_free(image);

  return texture;
}

void TextureManager::UpdateAudioTexture(const std::vector<GLubyte>& data)
{
  if (m_channelTextures[0].IsValid())
  {
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, m_channelTextures[0].GetID());
    glTexImage2D(GL_TEXTURE_2D,
                 0,
                 GL_RED,
                 static_cast<GLsizei>(AudioProcessor::NUM_BANDS),
                 2,
                 0,
                 GL_RED,
                 GL_UNSIGNED_BYTE,
                 data.data());
    glBindTexture(GL_TEXTURE_2D, 0);
  }
}

void TextureManager::ClearAll()
{
  for (int i = 0; i < MAX_CHANNELS; i++)
  {
    m_channelTextures[i].Release();
  }
  m_framebufferTexture.Release();
}

void TextureManager::SetFramebufferDimensions(int width, int height)
{
  m_fbWidth = width;
  m_fbHeight = height;
}

void TextureManager::CreateFramebuffer(int width, int height)
{
  m_fbWidth = width;
  m_fbHeight = height;

  // Create framebuffer texture
  glActiveTexture(GL_TEXTURE0);
  GLuint textureId = 0;
  glGenTextures(1, &textureId);
  glBindTexture(GL_TEXTURE_2D, textureId);
  glTexImage2D(GL_TEXTURE_2D,
               0,
               GL_RGB,
               width,
               height,
               0,
               GL_RGB,
               GL_UNSIGNED_BYTE,
               nullptr);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_2D, 0);
  m_framebufferTexture = Texture(textureId);

  // Create framebuffer object
  if (m_effectFb != 0)
  {
    glDeleteFramebuffers(1, &m_effectFb);
  }
  glGenFramebuffers(1, &m_effectFb);
  glBindFramebuffer(GL_FRAMEBUFFER, m_effectFb);
  glFramebufferTexture2D(GL_FRAMEBUFFER,
                         GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_2D,
                         m_framebufferTexture.GetID(),
                         0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void TextureManager::BindFramebuffer() const
{
  glBindFramebuffer(GL_FRAMEBUFFER, m_effectFb);
}

void TextureManager::UnbindFramebuffer()
{
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
