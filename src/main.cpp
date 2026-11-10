/*
 *  Copyright (C) 2005-2021 Team Kodi <https://kodi.tv>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 *  See LICENSE.md for more information.
 */

#include "main.h"
#include "math_utils.h"

#include <regex>

// Override GL_RED if not present with GL_LUMINANCE, e.g. on Android GLES
#ifndef GL_RED
#define GL_RED GL_LUMINANCE
#endif

// Common shader functions
static const std::string fsCommonFunctionsLowPower =
  R"functions(float h11(float p)
{
  return fract(.13 * p + 217943.37373737 / (p + 0.31));
}

float waveform(vec2 uv)
{
  float wave = texture(iChannel0,vec2(uv.x*.15+.5,0.75)).x - .5;
  return min(abs(uv.y*20.+wave*10.),0.5);
}

#ifdef dNoise
float noise(vec2 gv)
{
\treturn texture(iChannel2, vec2(gl_FragCoord.xy/(256.*cDotSize))).x;
}
#endif

vec3 bw2col(float bw, vec2 uv)
{
  float d = length(fract(uv*cColumns)-.5);
  float peakcolor = .6-d;
  float basecolor = .8-d;
  return (basecolor*cColor+peakcolor)*bw;
}

vec2 getUV()
{
  vec2 uv = (gl_FragCoord.xy-0.5*cResolution.xy)/cResolution.y;
  return uv;
}

)functions";

static const std::string fsCommonFunctionsNormal =
  R"functions(float h11(float p)
{
  return fract(20.12345+sin(p*cRNDSEED1)*cRNDSEED2);
}

float waveform(vec2 uv)
{
  float wave = texture(iChannel0,vec2(uv.x*.15+.5,0.75)).x*.5 + uv.y;
  return abs(smoothstep(.225,.275,wave) -.5);
}

#ifdef dNoise
float noise(vec2 gv)
{
  return texture(iChannel2, (gv*.035431) + iTime*cNoiseFluctuation).x;
}
#endif

vec3 bw2col(float bw, vec2 uv)
{
  float d = length(fract(uv*cColumns)-.5);
  float peakcolor = smoothstep(.35,.0,d)*bw;
  float basecolor = smoothstep(.85,.0,d)*bw;
  return basecolor*cColor+peakcolor;
}

#ifdef dCrtCurve
vec2 getUV()
{
  vec2 uv = (gl_FragCoord.xy-0.5*cResolution.xy)/cResolution.y;
  uv = uv / (1.00 - length(uv*.1));
  return uv;
}
#else
vec2 getUV()
{
  vec2 uv = (gl_FragCoord.xy-0.5*cResolution.xy)/cResolution.y;
  return uv;
}
#endif

)functions";

CVisualizationMatrix::CVisualizationMatrix()
    : m_renderer(m_textureManager)
{
  m_currentPreset = kodi::GetSettingInt("lastpresetidx");
  m_dotMode = static_cast<float>(kodi::GetSettingBoolean("dotmode"));
  if (m_dotMode)
  {
    m_dotSize = static_cast<float>(kodi::GetSettingInt("dotsize"));
  }
  else
  {
    if (Height() <= 900)
      m_dotSize = 3.0f;
    else if (Height() <= 1500)
      m_dotSize = 4.0f;
    else
      m_dotSize = 5.0f;
  }
  m_fallSpeed = static_cast<float>(kodi::GetSettingInt("fallspeed")) * 0.01f;
  m_distortThreshold = static_cast<float>(kodi::GetSettingInt("distortthreshold")) * 0.005f;
  m_rainHighlights = static_cast<float>(kodi::GetSettingInt("rainhighlights")) * 0.016f;
  m_dotColor.red = static_cast<float>(kodi::GetSettingInt("red")) / 255.0f;
  m_dotColor.green = static_cast<float>(kodi::GetSettingInt("green")) / 255.0f;
  m_dotColor.blue = static_cast<float>(kodi::GetSettingInt("blue")) / 255.0f;
  m_lowpower = kodi::GetSettingBoolean("lowpower");
  m_noiseFluctuation = m_lowpower
                         ? (static_cast<float>(kodi::GetSettingInt("noisefluctuation")) * 0.0002f) /
                             m_fallSpeed * 0.25f
                         : (static_cast<float>(kodi::GetSettingInt("noisefluctuation")) * 0.0004f) /
                             m_fallSpeed * 0.25f;
  m_crtCurve = kodi::GetSettingBoolean("crtcurve");
  m_lastAlbumChange = 0.0;
  m_audioProcessor.SetSamplesPerSec(0);
}

CVisualizationMatrix::~CVisualizationMatrix()
{
  // RAII objects clean up automatically
}

//-- Render -------------------------------------------------------------------
// Called once per frame. Do all rendering here.
//-----------------------------------------------------------------------------
void CVisualizationMatrix::Render()
{
  if (m_initialized)
  {
    const auto now = std::chrono::high_resolution_clock::now();
    const auto duration = std::chrono::duration<double>(now.time_since_epoch());
    int64_t intt = static_cast<int64_t>(duration.count() * 1000.0 * m_fallSpeed) - m_initialTime;
    if (m_bitsPrecision)
      intt &= (1 << m_bitsPrecision) - 1;

    const float t = static_cast<float>(intt) / 1000.0f;

    // Handle album art updates
    if (m_AlbumNeedsUpload && m_presetManager.GetPreset(m_currentPreset).channel[3] == 2)
    {
      const auto logotimer = std::chrono::duration<double>(
                               std::chrono::high_resolution_clock::now().time_since_epoch())
                             .count();
      const float delta = static_cast<float>(logotimer - m_lastAlbumChange) * 0.6f;
      const GLfloat r = std::max(std::sin(delta), 0.0f) * 0.7f;
      const GLfloat g = std::max(std::sin(delta - 1.0f), 0.0f) * 0.7f;
      const GLfloat b = std::max(std::sin(delta - 2.0f), 0.0f) * 0.7f;
      m_renderer.SetAlbumUniforms(m_albumX, m_albumY, r, g, b);

      if (m_lastAlbumChange == 0.0)
      {
        m_renderer.SetAlbumUniforms(0.0f, 0.0f, 0.7f, 0.7f, 0.7f);
      }
      if (logotimer - m_lastAlbumChange >= 10.0)
      {
        m_albumX = static_cast<GLfloat>(
          std::fmod(logotimer * 1234.0, 1.0) *
            (static_cast<double>(Width()) / static_cast<double>(Height()) + 1.0) -
          1.0);
        m_albumY = static_cast<GLfloat>(std::fmod(logotimer * 7654.0, 1.0));
        m_lastAlbumChange = logotimer;
        m_AlbumNeedsUpload = true;
      }
      if (m_AlbumNeedsUpload)
      {
        m_albumX = std::max(-1.0f, std::min(1.0f, m_albumX));
        m_albumY = std::max(-1.0f, std::min(1.0f, m_albumY));
        m_renderer.SetAlbumUniforms(m_albumX, m_albumY, r, g, b);
        m_AlbumNeedsUpload = false;
      }
    }

    m_renderer.Render(m_audioProcessor, t, m_fallSpeed);
  }
}

bool CVisualizationMatrix::Start(int iChannels,
                                 int iSamplesPerSec,
                                 int iBitsPerSample,
                                 std::string szSongName)
{
  kodi::Log(ADDON_LOG_DEBUG,
            "Start %i %i %i %s\n",
            iChannels,
            iSamplesPerSec,
            iBitsPerSample,
            szSongName.c_str());

  m_samplesPerSec = iSamplesPerSec;
  m_audioProcessor.SetSamplesPerSec(iSamplesPerSec);

  // Initialize renderer with vertex buffer
  m_renderer.Initialize(Width(), Height());

  Launch(m_currentPreset);
  m_initialized = true;

  return true;
}

void CVisualizationMatrix::Stop()
{
  m_initialized = false;
  kodi::Log(ADDON_LOG_DEBUG, "Stop");

  m_textureManager.ClearAll();
}

void CVisualizationMatrix::AudioData(const float* pAudioData,
                                     int iAudioDataLength,
                                     float* pFreqData,
                                     int iFreqDataLength)
{
  m_audioProcessor.ProcessAudioData(pAudioData, iAudioDataLength, 2);
}

//-- OnAction -----------------------------------------------------------------
// Handle Kodi actions such as next preset, lock preset, album art changed etc
//-----------------------------------------------------------------------------
bool CVisualizationMatrix::NextPreset()
{
  m_currentPreset = m_presetManager.GetNextPreset(m_currentPreset);
  Launch(m_currentPreset);
  UpdateAlbumart();
  kodi::SetSettingInt("lastpresetidx", m_currentPreset);
  return true;
}

bool CVisualizationMatrix::PrevPreset()
{
  m_currentPreset = m_presetManager.GetPrevPreset(m_currentPreset);
  Launch(m_currentPreset);
  UpdateAlbumart();
  kodi::SetSettingInt("lastpresetidx", m_currentPreset);
  return true;
}

bool CVisualizationMatrix::LoadPreset(int select)
{
  kodi::Log(ADDON_LOG_DEBUG, "Loading preset %i\n", select);
  m_currentPreset = select % static_cast<int>(m_presetManager.GetPresetCount());
  Launch(m_currentPreset);
  UpdateAlbumart();
  kodi::SetSettingInt("lastpresetidx", m_currentPreset);
  return true;
}

bool CVisualizationMatrix::RandomPreset()
{
  m_currentPreset = m_presetManager.GetRandomPreset();
  Launch(m_currentPreset);
  UpdateAlbumart();
  kodi::SetSettingInt("lastpresetidx", m_currentPreset);
  return true;
}

//-- GetPresets ---------------------------------------------------------------
// Return a list of presets to Kodi for display
//-----------------------------------------------------------------------------
bool CVisualizationMatrix::GetPresets(std::vector<std::string>& presets)
{
  std::string name;
  for (const auto& preset : m_presetManager.GetPresets())
  {
    name = kodi::GetLocalizedString(preset.labelId, preset.name);
    presets.push_back(name);
  }
  return true;
}

//-- GetPreset ----------------------------------------------------------------
// Return the index of the current playing preset
//-----------------------------------------------------------------------------
int CVisualizationMatrix::GetActivePreset()
{
  return m_currentPreset;
}

bool CVisualizationMatrix::UpdateAlbumart()
{
  return UpdateAlbumart(m_albumArt);
}

bool CVisualizationMatrix::UpdateAlbumart(std::string albumart)
{
  m_albumArt = std::move(albumart);

  kodi::Log(ADDON_LOG_DEBUG, "Updating album art %s\n", m_albumArt.c_str());
  if (m_presetManager.GetPreset(m_currentPreset).channel[3] != 2)
  {
    return false;
  }

  std::string thumb = kodi::vfs::GetCacheThumbName(m_albumArt.c_str());
  thumb = thumb.substr(0, 8);
  std::string special = std::string("special://thumbnails/") + thumb[0] + std::string("/") + thumb;

  if (kodi::vfs::FileExists(special + std::string(".png")))
  {
    m_shaderTextures[3].texture =
      kodi::vfs::TranslateSpecialProtocol(special + std::string(".png"));
    return true;
  }
  else if (kodi::vfs::FileExists(special + std::string(".jpg")))
  {
    m_shaderTextures[3].texture =
      kodi::vfs::TranslateSpecialProtocol(special + std::string(".jpg"));
    return true;
  }

  m_shaderTextures[3].texture = kodi::GetAddonPath("resources/textures/logo.png");
  return false;
}

void CVisualizationMatrix::Launch(int preset)
{
  m_bitsPrecision = DetermineBitsPrecision();
  m_bitsPrecision = std::max(m_bitsPrecision, 13);
  kodi::Log(ADDON_LOG_DEBUG, "bits of precision: %d", m_bitsPrecision);
  m_renderer.SetBitsPrecision(m_bitsPrecision);

  m_textureManager.ClearAll();

  // Reset album upload flag - only set to true for album shader
  m_AlbumNeedsUpload = (m_presetManager.GetPreset(preset).channel[3] == 2);

  m_usedShaderFile = kodi::GetAddonPath("resources/shaders/" + m_presetManager.GetPreset(preset).file);
  
  const auto& presetData = m_presetManager.GetPreset(preset);
  for (int i = 0; i < 4; i++)
  {
    if (presetData.channel[i] >= 0 &&
        presetData.channel[i] < static_cast<int>(m_presetManager.GetFileTextures().size()))
    {
      m_shaderTextures[i].texture =
        kodi::GetAddonPath("resources/textures/" + m_presetManager.GetFileTextures()[presetData.channel[i]]);
    }
    else if (presetData.channel[i] == 99)  // framebuffer
    {
      m_shaderTextures[i].audio = true;
    }
    else
    {
      m_shaderTextures[i].texture.clear();
      m_shaderTextures[i].audio = false;
    }
  }

  // Create audio texture (channel 0)
  m_textureManager.GetChannelTexture(0) =
    m_textureManager.CreateTexture(GL_RED, 
                                     static_cast<unsigned int>(AudioProcessor::NUM_BANDS), 
                                     2, 
                                     m_audioProcessor.GetAudioTextureData().data());
  
  // Logo texture (channel 1)
  if (!m_shaderTextures[1].texture.empty())
  {
    m_textureManager.GetChannelTexture(1) =
      m_textureManager.CreateTexture(m_shaderTextures[1].texture, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
  }
  
  // Noise texture (channel 2)
  if (!m_shaderTextures[2].texture.empty())
  {
    m_textureManager.GetChannelTexture(2) =
      m_textureManager.CreateTexture(m_shaderTextures[2].texture, GL_RGBA, GL_LINEAR, GL_REPEAT);
  }
  
  // Album texture (channel 3)
  if (!m_shaderTextures[3].texture.empty())
  {
    m_textureManager.GetChannelTexture(3) =
      m_textureManager.CreateTexture(m_shaderTextures[3].texture, GL_RGBA, GL_LINEAR, GL_CLAMP_TO_EDGE);
  }

  m_textureManager.SetFramebufferDimensions(Width(), Height());
  LoadPreset(m_usedShaderFile);
}

void CVisualizationMatrix::LoadPreset(const std::string& shaderPath)
{
  GatherDefines();
  m_renderer.LoadShader(shaderPath, m_defines);

  const auto now = std::chrono::high_resolution_clock::now();
  const auto duration = std::chrono::duration<double>(now.time_since_epoch());
  m_initialTime = static_cast<int64_t>(duration.count() * 1000.0);
  m_initialTime += (m_initialTime % 100000);
}

void CVisualizationMatrix::UnloadPreset()
{
  m_renderer.UnloadShader();
}

int CVisualizationMatrix::DetermineBitsPrecision()
{
  m_textureManager.SetFramebufferDimensions(32, 26 * 10);
  LoadPreset(kodi::GetAddonPath("resources/shaders/main_test.frag.glsl"));
  
  // Create temporary framebuffer for testing
  m_textureManager.CreateFramebuffer(32, 260);
  
  // Render test pattern
  const auto now = std::chrono::high_resolution_clock::now();
  const auto duration = std::chrono::duration<double>(now.time_since_epoch());
  const float t = static_cast<float>(duration.count() * 1000.0 * m_fallSpeed) - static_cast<float>(m_initialTime);
  m_renderer.Render(m_audioProcessor, t, m_fallSpeed);
  glFinish();

  std::unique_ptr<unsigned char[]> buffer(new unsigned char[32 * 260 * 4]);
  if (buffer)
    glReadPixels(0, 0, 32, 260, GL_RGBA, GL_UNSIGNED_BYTE, buffer.get());

  int bits = 0;
  unsigned char b = 0;
  for (int j = 0; j < 260; j++)
  {
    const unsigned char c = buffer[4 * (j * 32 + (32 >> 1))];
    if (c && !b)
      bits++;
    b = c;
  }
  UnloadPreset();
  return bits;
}

void CVisualizationMatrix::GatherDefines()
{
  m_defines.clear();
#if defined(HAS_GL)
  m_defines += "#version 150\n";
  m_defines += "#extension GL_OES_standard_derivatives : enable\n";
  m_defines += "out vec4 FragColor;\n";
  m_defines += "#ifndef texture2D\n#define texture2D texture\n#endif\n\n";
#else
  m_defines += "#version 100\n\n";
  m_defines += "#extension GL_OES_standard_derivatives : enable\n\n";
  m_defines += "precision mediump float;\n";
  m_defines += "precision mediump int;\n\n";
  m_defines += "#define FragColor gl_FragColor\n";
  m_defines += "#ifndef texture\n#define texture texture2D\n#endif\n\n";
#endif

  m_defines += "const float cRNDSEED1 = 170.12;\n";
  m_defines += "const float cRNDSEED2 = 7572.1;\n";
  m_defines += "const float cINTENSITY = 1.0;\n";
  m_defines += "const float cMININTENSITY = 0.075;\n";
  m_defines += "const float cDISTORTFACTORX = 0.6;\n";
  m_defines += "const float cDISTORTFACTORY = 0.4;\n";
  m_defines += "const float cVIGNETTEINTENSITY = 0.05;\n";

  m_defines += "const float cDotSize = " + std::to_string(m_dotSize) + ";\n";
  m_defines +=
    "const float cColumns = " + std::to_string(static_cast<float>(Width()) / (m_dotSize * 2.0f)) +
    ";\n";
  m_defines += "const float cNoiseFluctuation = " + std::to_string(m_noiseFluctuation) + ";\n";
  m_defines += "const float cDistortThreshold = " + std::to_string(m_distortThreshold) + ";\n";
  m_defines += "const float cRainHighlights = " + std::to_string(m_rainHighlights) + ";\n";
  m_defines += "const vec3 cColor = vec3(" + std::to_string(m_dotColor.red) + "," +
               std::to_string(m_dotColor.green) + "," + std::to_string(m_dotColor.blue) + ");\n";

  auto fbDims = m_textureManager.GetFramebufferDimensions();
  if (fbDims.x > 0 && fbDims.y > 0)
  {
    m_defines += "const vec2 cResolution = vec2(" + std::to_string(fbDims.x) + "," +
                 std::to_string(fbDims.y) + ");\n";
  }
  else
  {
    m_defines += "const vec2 cResolution = vec2(" + std::to_string(Width()) + ".," +
                 std::to_string(Height()) + ".);\n";
  }

  m_defines += "uniform sampler2D iChannel0;\n";

  const auto& preset = m_presetManager.GetPreset(m_currentPreset);
  if (preset.channel[1] != -1)
  {
    m_defines += "uniform sampler2D iChannel1;\n";
  }

  if (preset.channel[2] != -1)
  {
    m_defines += "uniform sampler2D iChannel2;\n";
    m_defines += "#define dNoise\n";
  }

  if (preset.channel[3] != -1)
  {
    m_defines += "uniform sampler2D iChannel3;\n";
  }

  if (preset.channel[3] == 2)
  {
    m_defines += "uniform vec3 iAlbumPosition;\n";
    m_defines += "uniform vec3 iAlbumRGB;\n";
  }

  if (m_crtCurve)
  {
    m_defines += "#define dCrtCurve\n";
  }

  m_defines += "uniform float iTime;\n";

  if (m_lowpower)
  {
    m_defines += fsCommonFunctionsLowPower;
  }
  else
  {
    m_defines += fsCommonFunctionsNormal;
  }

  kodi::Log(ADDON_LOG_DEBUG, "Fragment shader header\n%s", m_defines.c_str());
}

ADDONCREATOR(CVisualizationMatrix)  // Don't touch this!
