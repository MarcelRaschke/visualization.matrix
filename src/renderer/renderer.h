/*
 *  Copyright (C) 2005-2021 Team Kodi <https://kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <glm/glm.hpp>
#include <kodi/gui/gl/Shader.h>
#include <memory>
#include <string>

class AudioProcessor;
class TextureManager;

/**
 * @brief Handles all rendering operations for the visualization.
 */
class Renderer
{
public:
  /**
   * @brief Construct a new Renderer object.
   * @param textureManager Reference to texture manager
   */
  explicit Renderer(TextureManager& textureManager);

  /**
   * @brief Destroy the Renderer object.
   */
  ~Renderer();

  // Disable copy and move
  Renderer(const Renderer&) = delete;
  Renderer& operator=(const Renderer&) = delete;
  Renderer(Renderer&&) = delete;
  Renderer& operator=(Renderer&&) = delete;

  /**
   * @brief Initialize the renderer.
   * @param width Display width
   * @param height Display height
   */
  void Initialize(int width, int height);

  /**
   * @brief Render a frame.
   * @param audioProcessor Reference to audio processor
   * @param globalTime Global time in seconds
   * @param fallSpeed Fall speed multiplier
   */
  void Render(AudioProcessor& audioProcessor, float globalTime, float fallSpeed);

  /**
   * @brief Load a shader preset.
   * @param shaderPath Path to fragment shader
   * @param defines Shader defines
   * @return true if successful
   */
  bool LoadShader(const std::string& shaderPath, const std::string& defines);

  /**
   * @brief Unload current shader.
   */
  void UnloadShader();

  /**
   * @brief Set shader uniforms for album art.
   * @param albumX X position
   * @param albumY Y position
   * @param r Red component
   * @param g Green component
   * @param b Blue component
   */
  void SetAlbumUniforms(float albumX, float albumY, float r, float g, float b);

  /**
   * @brief Get the shader program handle.
   * @return GLuint Shader program handle
   */
  GLuint GetShaderProgram() const;

  /**
   * @brief Set view dimensions.
   * @param width Width
   * @param height Height
   */
  void SetViewDimensions(int width, int height);

  /**
   * @brief Update time precision bits.
   * @param bitsPrecision Number of bits
   */
  void SetBitsPrecision(int bitsPrecision);

private:
  TextureManager& m_textureManager;
  kodi::gui::gl::CShaderProgram m_shader;
  std::string m_currentShaderPath;

  // Uniform locations
  GLint m_attrGlobalTimeLoc = 0;
  GLint m_attrAlbumPositionLoc = 0;
  GLint m_attrAlbumRGBLoc = 0;
  GLint m_attrChannelLoc[4] = {0};

  // Vertex buffer
  GLuint m_vertexBuffer = 0;
  GLint m_attrVertex = 0;

  // State
  int m_bitsPrecision = 0;
  int m_width = 0;
  int m_height = 0;

  /**
   * @brief Create vertex buffer for full-screen quad.
   */
  void CreateVertexBuffer();

  /**
   * @brief Setup shader uniforms.
   */
  void SetupUniforms();

  /**
   * @brief Bind textures for rendering.
   */
  void BindTextures();

  /**
   * @brief Render full-screen quad.
   */
  void RenderQuad();
};
