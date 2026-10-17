/*
 *  Copyright (C) 2005-2021 Team Kodi <https://kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "renderer.h"
#include "../audio/audio_processor.h"
#include "../textures/texture_manager.h"

#include <chrono>

Renderer::Renderer(TextureManager& textureManager)
    : m_textureManager(textureManager)
{
}

Renderer::~Renderer()
{
  UnloadShader();
  if (m_vertexBuffer != 0)
  {
    glDeleteBuffers(1, &m_vertexBuffer);
    m_vertexBuffer = 0;
  }
}

void Renderer::Initialize(int width, int height)
{
  m_width = width;
  m_height = height;
  CreateVertexBuffer();
}

void Renderer::CreateVertexBuffer()
{
  // Background vertex (full-screen quad)
  static const GLfloat vertexData[] = {
    -1.0f, 1.0f, 1.0f, 1.0f,  // top-left
    1.0f,  1.0f, 1.0f, 1.0f,  // top-right
    1.0f,  1.0f, 1.0f, 1.0f,  // top-right
    1.0f, -1.0f, 1.0f, 1.0f,  // bottom-right
    1.0f, -1.0f, 1.0f, 1.0f,  // bottom-right
    -1.0f, -1.0f, 1.0f, 1.0f,  // bottom-left
    -1.0f, -1.0f, 1.0f, 1.0f,  // bottom-left
    -1.0f, 1.0f, 1.0f, 1.0f,  // top-left
  };

  glGenBuffers(1, &m_vertexBuffer);
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void Renderer::Render(AudioProcessor& audioProcessor, float globalTime, float fallSpeed)
{
  if (!m_shader.ProgramHandle())
    return;

  glUseProgram(m_shader.ProgramHandle());

  // Update audio textures if needed
  if (audioProcessor.NeedsUpload())
  {
    m_textureManager.UpdateAudioTexture(audioProcessor.GetAudioTextureData());
    audioProcessor.ResetUploadFlag();
  }

  // Set global time uniform
  glUniform1f(m_attrGlobalTimeLoc, globalTime);

  // Bind textures
  BindTextures();

  // Set album uniforms if available
  if (m_attrAlbumPositionLoc != 0 && m_attrAlbumRGBLoc != 0)
  {
    // Default album position and color
    glUniform3f(m_attrAlbumPositionLoc, 0.0f, 0.0f, 2.0f);
    glUniform3f(m_attrAlbumRGBLoc, 0.7f, 0.7f, 0.7f);
  }

  // Draw to framebuffer
  m_textureManager.BindFramebuffer();

  // Render quad
  RenderQuad();

  // Unbind framebuffer
  TextureManager::UnbindFramebuffer();

  glUseProgram(0);
}

void Renderer::BindTextures()
{
  for (int i = 0; i < 4; i++)
  {
    glActiveTexture(GL_TEXTURE0 + i);
    glUniform1i(m_attrChannelLoc[i], i);
    glBindTexture(GL_TEXTURE_2D, m_textureManager.GetChannelTexture(i).GetID());
  }
}

void Renderer::RenderQuad()
{
  glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
  glVertexAttribPointer(m_attrVertex, 4, GL_FLOAT, GL_FALSE, 16, nullptr);
  glEnableVertexAttribArray(m_attrVertex);
  glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
  glDisableVertexAttribArray(m_attrVertex);
  glBindBuffer(GL_ARRAY_BUFFER, 0);

  // Clean up texture bindings
  for (int i = 0; i < 4; i++)
  {
    glActiveTexture(GL_TEXTURE0 + i);
    glBindTexture(GL_TEXTURE_2D, 0);
  }
}

bool Renderer::LoadShader(const std::string& shaderPath, const std::string& defines)
{
  UnloadShader();

  std::string vertShaderPath;
#if defined(HAS_GL)
  vertShaderPath = kodi::GetAddonPath("resources/shaders/main_matrix_GL.vert.glsl");
#else
  vertShaderPath = kodi::GetAddonPath("resources/shaders/main_matrix_GLES.vert.glsl");
#endif

  if (!m_shader.LoadShaderFiles(vertShaderPath, shaderPath) ||
      !m_shader.CompileAndLink("", "", defines, ""))
  {
    kodi::Log(ADDON_LOG_ERROR, "Failed to compile shaders (file '%s')", shaderPath.c_str());
    return false;
  }

  m_currentShaderPath = shaderPath;
  SetupUniforms();

  return true;
}

void Renderer::SetupUniforms()
{
  const GLuint program = m_shader.ProgramHandle();

  m_attrGlobalTimeLoc = glGetUniformLocation(program, "iTime");
  m_attrAlbumPositionLoc = glGetUniformLocation(program, "iAlbumPosition");
  m_attrAlbumRGBLoc = glGetUniformLocation(program, "iAlbumRGB");
  m_attrChannelLoc[0] = glGetUniformLocation(program, "iChannel0");
  m_attrChannelLoc[1] = glGetUniformLocation(program, "iChannel1");
  m_attrChannelLoc[2] = glGetUniformLocation(program, "iChannel2");
  m_attrChannelLoc[3] = glGetUniformLocation(program, "iChannel3");

  m_attrVertex = glGetAttribLocation(program, "vertex");
}

void Renderer::UnloadShader()
{
  m_shader.~CShaderProgram();
  new (&m_shader) kodi::gui::gl::CShaderProgram();
  m_currentShaderPath.clear();
  m_attrGlobalTimeLoc = 0;
  m_attrAlbumPositionLoc = 0;
  m_attrAlbumRGBLoc = 0;
  for (int i = 0; i < 4; i++)
    m_attrChannelLoc[i] = 0;
}

GLuint Renderer::GetShaderProgram() const
{
  return m_shader.ProgramHandle();
}

void Renderer::SetViewDimensions(int width, int height)
{
  m_width = width;
  m_height = height;
}

void Renderer::SetBitsPrecision(int bitsPrecision)
{
  m_bitsPrecision = bitsPrecision;
}

void Renderer::SetAlbumUniforms(float albumX, float albumY, float r, float g, float b)
{
  if (m_attrAlbumPositionLoc != 0)
    glUniform3f(m_attrAlbumPositionLoc, albumX, albumY, 2.0f);
  if (m_attrAlbumRGBLoc != 0)
    glUniform3f(m_attrAlbumRGBLoc, r, g, b);
}
