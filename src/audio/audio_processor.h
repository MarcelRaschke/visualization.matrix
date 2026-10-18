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

#include <kissfft/kiss_fft.h>
#include <vector>
#include <memory>

/**
 * @brief Handles all audio processing including FFT analysis and magnitude calculation.
 */
class AudioProcessor
{
public:
  static constexpr size_t AUDIO_BUFFER = 1024;
  static constexpr size_t NUM_BANDS = AUDIO_BUFFER / 2;
  static constexpr float SMOOTHING_TIME_CONSTANT = 0.5f;
  static constexpr float MIN_DECIBELS = -100.0f;
  static constexpr float MAX_DECIBELS = -30.0f;

  /**
   * @brief Construct a new Audio Processor object.
   */
  AudioProcessor();

  /**
   * @brief Destroy the Audio Processor object.
   */
  ~AudioProcessor();

  // Disable copy and move
  AudioProcessor(const AudioProcessor&) = delete;
  AudioProcessor& operator=(const AudioProcessor&) = delete;
  AudioProcessor(AudioProcessor&&) = delete;
  AudioProcessor& operator=(AudioProcessor&&) = delete;

  /**
   * @brief Process new audio data and compute FFT.
   * @param pAudioData Pointer to audio data
   * @param iAudioDataLength Length of audio data
   */
  void ProcessAudioData(const float* pAudioData, int iAudioDataLength, int channels);

  /**
   * @brief Get the magnitude buffer for visualization.
   * @return const std::vector<GLubyte>& Audio data as texture-ready bytes
   */
  const std::vector<GLubyte>& GetAudioTextureData() const { return m_audioData; }

  /**
   * @brief Get the PCM buffer.
   * @return const std::vector<float>& PCM data
   */
  const std::vector<float>& GetPCMData() const { return m_pcm; }

  /**
   * @brief Get samples per second.
   * @return int Samples per second
   */
  int GetSamplesPerSec() const { return m_samplesPerSec; }

  /**
   * @brief Set samples per second.
   * @param samplesPerSec Samples per second
   */
  void SetSamplesPerSec(int samplesPerSec) { m_samplesPerSec = samplesPerSec; }

  /**
   * @brief Check if new data is available for upload.
   * @return true if data needs upload
   */
  bool NeedsUpload() const { return m_needsUpload; }

  /**
   * @brief Reset upload flag.
   */
  void ResetUploadFlag() { m_needsUpload = false; }

private:
  kiss_fft_cfg m_kissCfg;
  std::vector<GLubyte> m_audioData;
  std::vector<float> m_magnitudeBuffer;
  std::vector<float> m_pcm;
  int m_samplesPerSec = 0;
  bool m_needsUpload = false;

  /**
   * @brief Mix audio channels into mono.
   * @param destination Destination buffer
   * @param source Source buffer
   * @param frames Number of frames
   * @param channels Number of channels
   */
  void Mix(float* destination, const float* source, size_t frames, size_t channels);

  /**
   * @brief Write audio data to internal buffer.
   * @param input Input audio data
   * @param length Length of input data
   * @param channels Number of channels
   */
  void WriteToBuffer(const float* input, size_t length, size_t channels);
};
