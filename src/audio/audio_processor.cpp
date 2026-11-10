/*
 *  Copyright (C) 2005-2021 Team Kodi <https://kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 *
 *  DCO: I certify that this contribution is licensed under the GPL-2.0-or-later
 *       and I have the right to submit it under this license.
 */

#include "audio_processor.h"
#include "../math_utils.h"

#include <algorithm>
#include <cmath>

AudioProcessor::AudioProcessor()
    : m_kissCfg(kiss_fft_alloc(static_cast<int>(AUDIO_BUFFER), 0, nullptr, nullptr)),
      m_audioData(AUDIO_BUFFER),
      m_magnitudeBuffer(NUM_BANDS),
      m_pcm(AUDIO_BUFFER)
{
}

AudioProcessor::~AudioProcessor()
{
  free(m_kissCfg);
}

void AudioProcessor::ProcessAudioData(const float* pAudioData, int iAudioDataLength, int channels)
{
  WriteToBuffer(pAudioData, static_cast<size_t>(iAudioDataLength), static_cast<size_t>(channels));

  std::vector<kiss_fft_cpx> in(AUDIO_BUFFER);
  std::vector<kiss_fft_cpx> out(AUDIO_BUFFER);

  for (size_t i = 0; i < AUDIO_BUFFER; i++)
  {
    in[i].r = BlackmanWindow(m_pcm[i], i, AUDIO_BUFFER);
    in[i].i = 0;
  }

  kiss_fft(m_kissCfg, in.data(), out.data());

  out[0].i = 0;

  // Create temporary copy for smoothing
  std::vector<float> tempBuffer = m_magnitudeBuffer;
  SmoothingOverTime(m_magnitudeBuffer,
                    tempBuffer,
                    out.data(),
                    NUM_BANDS,
                    SMOOTHING_TIME_CONSTANT,
                    static_cast<unsigned int>(AUDIO_BUFFER));

  const double rangeScaleFactor =
    MAX_DECIBELS == MIN_DECIBELS ? 1 : (1.0 / (MAX_DECIBELS - MIN_DECIBELS));

  for (size_t i = 0; i < NUM_BANDS; i++)
  {
    float linearValue = m_magnitudeBuffer[i];
    double dbMag = !linearValue ? MIN_DECIBELS : LinearToDecibels(linearValue);
    double scaledValue = UCHAR_MAX * (dbMag - MIN_DECIBELS) * rangeScaleFactor;

    m_audioData[i] =
      static_cast<GLubyte>(std::max(std::min(static_cast<int>(scaledValue), UCHAR_MAX), 0));
  }

  for (size_t i = 0; i < NUM_BANDS; i++)
  {
    float v = (m_pcm[i] + 1.0f) * 128.0f;
    m_audioData[i + NUM_BANDS] =
      static_cast<GLubyte>(std::max(std::min(static_cast<int>(v), UCHAR_MAX), 0));
  }

  m_needsUpload = true;
}

void AudioProcessor::Mix(float* destination,
                         const float* source,
                         size_t frames,
                         size_t channels)
{
  const size_t length = frames * channels;
  for (size_t i = 0; i < length; i += channels)
  {
    float v = 0.0f;
    for (size_t j = 0; j < channels; j++)
    {
      v += source[i + j];
    }

    destination[(i / 2)] = v / static_cast<float>(channels);
  }
}

void AudioProcessor::WriteToBuffer(const float* input, size_t length, size_t channels)
{
  const size_t frames = length / channels;

  if (frames >= AUDIO_BUFFER)
  {
    const size_t offset = frames - AUDIO_BUFFER;

    Mix(m_pcm.data(), input + offset, AUDIO_BUFFER, channels);
  }
  else
  {
    const size_t keep = AUDIO_BUFFER - frames;
    std::copy(m_pcm.begin() + frames, m_pcm.end(), m_pcm.begin());

    Mix(m_pcm.data() + keep, input, frames, channels);
  }
}
