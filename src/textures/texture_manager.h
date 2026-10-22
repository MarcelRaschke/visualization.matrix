/*
 *  Copyright (C) 2005-2021 Team Kodi <https://kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 *
 *  DCO: I certify that this contribution is licensed under the GPL-2.0-or-later
 *       and I have the right to submit it under this license.
 */

#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>

/**
 * @brief RAII wrapper for OpenGL texture objects.
 */
class Texture
{
public:
  /**
   * @brief Construct a new Texture object.
   */
  Texture() = default;

  /**
   * @brief Construct a new Texture object with parameters.
   * @param textureId OpenGL texture ID
   */
  explicit Texture(GLuint textureId) : m_textureId(textureId) {}

  /**
   * @brief Destroy the Texture object and free OpenGL resources.
   */
  ~Texture();

  // Disable copy, allow move
  Texture(const Texture&) = delete;
  Texture& operator=(const Texture&) = delete;
  Texture(Texture&& other) noexcept;
  Texture& operator=(Texture&& other) noexcept;

  /**
   * @brief Get the OpenGL texture ID.
   * @return GLuint Texture ID
   */
  GLuint GetID() const { return m_textureId; }

  /**
   * @brief Check if texture is valid.
   * @return true if texture is valid
   */
  bool IsValid() const { return m_textureId != 0; }

  /**
   * @brief Release the texture (free OpenGL resources).
   */
  void Release();

private:
  GLuint m_textureId = 0;
};

/**
 * @brief Manages all textures for the visualization.
 */
class TextureManager
{
public:
  static constexpr int MAX_CHANNELS = 4;

  /**
   * @brief Construct a new Texture Manager object.
   */
  TextureManager() = default;

  /**
   * @brief Destroy the Texture Manager object.
   */
  ~TextureManager();

  // Disable copy and move
  TextureManager(const TextureManager&) = delete;
  TextureManager& operator=(const TextureManager&) = delete;
  TextureManager(TextureManager&&) = delete;
  TextureManager& operator=(TextureManager&&) = delete;

  /**
   * @brief Create a texture from raw data.
   * @param format Texture format
   * @param w Width
   * @param h Height
   * @param data Pixel data
   * @return Texture New texture
   */
  Texture CreateTexture(GLint format, unsigned int w, unsigned int h, const GLvoid* data);

  /**
   * @brief Create a texture from file.
   * @param file File path
   * @param internalFormat Internal format
   * @param scaling Scaling filter
   * @param repeat Repeat mode
   * @return Texture New texture
   */
  Texture CreateTexture(const std::string& file,
                        GLint internalFormat,
                        GLint scaling,
                        GLint repeat);

  /**
   * @brief Create a texture with full parameters.
   * @param data Pixel data
   * @param format Format
   * @param w Width
   * @param h Height
   * @param internalFormat Internal format
   * @param scaling Scaling filter
   * @param repeat Repeat mode
   * @return Texture New texture
   */
  Texture CreateTexture(const GLvoid* data,
                        GLint format,
                        unsigned int w,
                        unsigned int h,
                        GLint internalFormat,
                        GLint scaling,
                        GLint repeat);

  /**
   * @brief Get channel texture.
   * @param index Channel index
   * @return Texture& Channel texture
   */
  Texture& GetChannelTexture(int index) { return m_channelTextures[index]; }

  /**
   * @brief Get const channel texture.
   * @param index Channel index
   * @return const Texture& Channel texture
   */
  const Texture& GetChannelTexture(int index) const { return m_channelTextures[index]; }

  /**
   * @brief Update audio texture data.
   * @param data Audio data
   */
  void UpdateAudioTexture(const std::vector<GLubyte>& data);

  /**
   * @brief Clear all textures.
   */
  void ClearAll();

  /**
   * @brief Get framebuffer texture.
   * @return Texture& Framebuffer texture
   */
  Texture& GetFramebufferTexture() { return m_framebufferTexture; }

  /**
   * @brief Get effect framebuffer.
   * @return GLuint Framebuffer ID
   */
  GLuint GetEffectFramebuffer() const { return m_effectFb; }

  /**
   * @brief Get framebuffer dimensions.
   * @return glm::ivec2 Dimensions
   */
  glm::ivec2 GetFramebufferDimensions() const { return {m_fbWidth, m_fbHeight}; }

  /**
   * @brief Set framebuffer dimensions.
   * @param width Width
   * @param height Height
   */
  void SetFramebufferDimensions(int width, int height);

  /**
   * @brief Create framebuffer texture and FBO.
   * @param width Width
   * @param height Height
   */
  void CreateFramebuffer(int width, int height);

  /**
   * @brief Bind framebuffer for rendering.
   */
  void BindFramebuffer() const;

  /**
   * @brief Unbind framebuffer.
   */
  static void UnbindFramebuffer();

private:
  Texture m_channelTextures[MAX_CHANNELS];
  Texture m_framebufferTexture;
  GLuint m_effectFb = 0;
  int m_fbWidth = 0;
  int m_fbHeight = 0;
};
