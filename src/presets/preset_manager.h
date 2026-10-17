/*
 *  Copyright (C) 2005-2021 Team Kodi <https://kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <string>
#include <vector>

/**
 * @brief Represents a visualization preset.
 */
struct Preset
{
  std::string name;
  uint32_t labelId;
  std::string file;
  int channel[4];
};

/**
 * @brief Manages all available presets.
 */
class PresetManager
{
public:
  /**
   * @brief Construct a new Preset Manager object.
   */
  PresetManager();

  /**
   * @brief Get all presets.
   * @return const std::vector<Preset>& List of presets
   */
  const std::vector<Preset>& GetPresets() const { return m_presets; }

  /**
   * @brief Get preset by index.
   * @param index Preset index
   * @return const Preset& Preset at index
   */
  const Preset& GetPreset(size_t index) const { return m_presets[index]; }

  /**
   * @brief Get number of presets.
   * @return size_t Number of presets
   */
  size_t GetPresetCount() const { return m_presets.size(); }

  /**
   * @brief Get preset index by name.
   * @param name Preset name
   * @return int Index or -1 if not found
   */
  int GetPresetIndex(const std::string& name) const;

  /**
   * @brief Get next preset index.
   * @param current Current preset index
   * @return int Next preset index
   */
  int GetNextPreset(int current) const;

  /**
   * @brief Get previous preset index.
   * @param current Current preset index
   * @return int Previous preset index
   */
  int GetPrevPreset(int current) const;

  /**
   * @brief Get a random preset index.
   * @return int Random preset index
   */
  int GetRandomPreset() const;

  /**
   * @brief Get file textures used by presets.
   * @return const std::vector<std::string>& List of file textures
   */
  const std::vector<std::string>& GetFileTextures() const { return m_fileTextures; }

private:
  std::vector<Preset> m_presets;
  std::vector<std::string> m_fileTextures;

  /**
   * @brief Initialize default presets.
   */
  void InitializePresets();
};
