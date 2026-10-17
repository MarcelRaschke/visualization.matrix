/*
 *  Copyright (C) 2005-2021 Team Kodi <https://kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "preset_manager.h"

#include <cstdlib>
#include <ctime>

PresetManager::PresetManager()
{
  InitializePresets();
  std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

void PresetManager::InitializePresets()
{
  // Default presets
  m_presets = {
    {"Kodi", 30100, "logo.frag.glsl", 99, 0, 1, -1},
    {"Album", 30101, "album.frag.glsl", 99, -1, 1, 2},
    {"Rain only", 30102, "nologo.frag.glsl", 99, -1, 1, -1},
    {"Rain with waveform", 30103, "nologowf.frag.glsl", 99, -1, 1, -1},
    {"Rain with waveform envelope", 30104, "nologowfenv.frag.glsl", 99, -1, 1, -1},
    {"Clean", 30105, "clean.frag.glsl", 99, -1, -1, -1},
    {"Clean with waveform", 30106, "cleanwf.frag.glsl", 99, -1, -1, -1},
    {"Clean with waveform envelope", 30107, "cleanwfenv.frag.glsl", 99, -1, -1, -1},
    {"Neon", 30108, "neon.frag.glsl", 99, -1, -1, -1},
    {"Odysseus", 30109, "odysseus.frag.glsl", 99, -1, -1, -1},
  };

  // File textures
  m_fileTextures = {
    "logo.png",
    "noise.png",
  };
}

int PresetManager::GetPresetIndex(const std::string& name) const
{
  for (size_t i = 0; i < m_presets.size(); i++)
  {
    if (m_presets[i].name == name)
      return static_cast<int>(i);
  }
  return -1;
}

int PresetManager::GetNextPreset(int current) const
{
  return (current + 1) % static_cast<int>(m_presets.size());
}

int PresetManager::GetPrevPreset(int current) const
{
  return (current - 1 + static_cast<int>(m_presets.size())) % static_cast<int>(m_presets.size());
}

int PresetManager::GetRandomPreset() const
{
  return static_cast<int>((std::rand() / static_cast<float>(RAND_MAX)) * m_presets.size());
}
