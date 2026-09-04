/*
 *  Copyright (C) 2005-2021 Team Kodi <https://kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#pragma once

#include <cmath>
#include <vector>

// FFT configuration constants
static constexpr size_t AUDIO_BUFFER = 1024;
static constexpr size_t NUM_BANDS = AUDIO_BUFFER / 2;

struct kiss_fft_cpx
{
  float r;
  float i;
};

/**
 * Apply Blackman window function to a sample.
 * The Blackman window reduces spectral leakage in FFT analysis.
 */
inline float BlackmanWindow(float in, size_t i, size_t length)
{
  constexpr double alpha = 0.16;
  constexpr double a0 = 0.5 * (1.0 - alpha);
  constexpr double a1 = 0.5;
  constexpr double a2 = 0.5 * alpha;

  const float x = static_cast<float>(i) / static_cast<float>(length);
  return in * static_cast<float>(a0 - a1 * std::cos(2.0 * M_PI * x) + a2 * std::cos(4.0 * M_PI * x));
}

/**
 * Convert linear amplitude to decibels.
 * Returns -1000 dB for zero input (effectively negative infinity).
 */
inline float LinearToDecibels(float linear)
{
  if (!linear)
    return -1000.0f;
  return 20.0f * std::log10(linear);
}

/**
 * Apply exponential smoothing to magnitude values over time.
 * This smooths the FFT output to reduce flickering in the visualization.
 */
inline void SmoothingOverTime(std::vector<float>& outputBuffer, const std::vector<float>& lastOutputBuffer,
                              kiss_fft_cpx* inputBuffer, size_t length, float smoothingTimeConstant, unsigned int fftSize)
{
  for (size_t i = 0; i < length; i++)
  {
    const kiss_fft_cpx c = inputBuffer[i];
    const float magnitude = std::sqrt(c.r * c.r + c.i * c.i) / static_cast<float>(fftSize);
    outputBuffer[i] = smoothingTimeConstant * lastOutputBuffer[i] + (1.0f - smoothingTimeConstant) * magnitude;
  }
}
