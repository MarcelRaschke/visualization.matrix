/*
 *  Copyright (C) 2005-2021 Team Kodi <https://kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <glm/ext.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <kodi/addon-instance/Visualization.h>

#include "audio/audio_processor.h"
#include "renderer/renderer.h"
#include "textures/texture_manager.h"
#include "presets/preset_manager.h"

#include <memory>
#include <string>
#include <vector>

class ATTRIBUTE_HIDDEN CVisualizationMatrix : public kodi::addon::CAddonBase,
                                              public kodi::addon::CInstanceVisualization
{
public:
  CVisualizationMatrix();
  ~CVisualizationMatrix() override;

  // Disable copy and move
  CVisualizationMatrix(const CVisualizationMatrix&) = delete;
  CVisualizationMatrix& operator=(const CVisualizationMatrix&) = delete;
  CVisualizationMatrix(CVisualizationMatrix&&) = delete;
  CVisualizationMatrix& operator=(CVisualizationMatrix&&) = delete;

  bool Start(int channels, int samplesPerSec, int bitsPerSample, std::string songName) override;
  void Stop() override;
  void AudioData(const float* audioData,
                 int audioDataLength,
                 float* freqData,
                 int freqDataLength) override;
  void Render() override;
  bool GetPresets(std::vector<std::string>& presets) override;
  int GetActivePreset() override;
  bool PrevPreset() override;
  bool NextPreset() override;
  bool LoadPreset(int select) override;
  bool RandomPreset() override;
  bool UpdateAlbumart(std::string albumart) override;

private:
  // Component managers
  AudioProcessor m_audioProcessor;
  TextureManager m_textureManager;
  Renderer m_renderer;
  PresetManager m_presetManager;

  // State
  bool m_initialized = false;
  int64_t m_initialTime = 0;  // in ms
  double m_lastAlbumChange = 0;
  int m_currentPreset = 0;
  bool m_AlbumNeedsUpload = true;
  float m_albumX = 0.0f;
  float m_albumY = 0.0f;

  // Settings
  float m_dotMode = false;
  float m_dotSize = 0.0f;
  float m_fallSpeed = 0.25f;
  float m_distortThreshold = 0.0f;
  float m_noiseFluctuation = 0.0f;
  float m_rainHighlights = 0.0f;
  bool m_lowpower = false;
  bool m_crtCurve = false;
  int m_bitsPrecision = 0;
  int m_samplesPerSec = 0;

  struct DotColor
  {
    float red = 0.0f;
    float green = 0.0f;
    float blue = 0.0f;
  } m_dotColor;

  std::string m_albumArt;
  std::string m_defines;
  std::string m_usedShaderFile;

  // Shader texture info
  struct ShaderTextureInfo
  {
    bool audio = false;
    std::string texture;
  } m_shaderTextures[4];

  // Preset channel info
  struct PresetChannelInfo
  {
    int channel[4];
  };

  /**
   * @brief Launch a preset.
   * @param preset Preset index
   */
  void Launch(int preset);

  /**
   * @brief Load a preset shader.
   * @param shaderPath Path to shader
   */
  void LoadPreset(const std::string& shaderPath);

  /**
   * @brief Unload current preset.
   */
  void UnloadPreset();

  /**
   * @brief Gather shader defines.
   */
  void GatherDefines();

  /**
   * @brief Update album art texture.
   * @param albumart Path to album art
   * @return true if updated
   */
  bool UpdateAlbumart();

  /**
   * @brief Determine bits precision for timer.
   * @return int Number of bits
   */
  int DetermineBitsPrecision();
};
