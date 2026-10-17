// Cyberpunk Matrix Preset
// GLSL Fragment Shader for visualization.matrix
// Features: Neon grid, scanning lines, glitch effects, audio-reactive

void main(void)
{
    // General stuff
    vec2 uv = getUV();

    // Time-based pulsing for cyberpunk feel
    float timePulse = sin(iTime * 0.3) * 0.2 + 0.8;

    // Cyberpunk grid background (dark with neon accents)
    vec2 grid = floor(uv * vec2(cColumns * 2.5, cColumns * 2.0));
    float gridLine = max(abs(fract(uv.x * cColumns * 2.5) - 0.5), 
                         abs(fract(uv.y * cColumns * 2.0) - 0.5));
    float gridIntensity = smoothstep(0.02, 0.0, gridLine) * 0.3;
    
    // Animated grid lines
    float gridPulse = sin(iTime * 0.5 + grid.x * 0.3 + grid.y * 0.4) * 0.5 + 0.5;
    vec3 gridColor = mix(vec3(0.05, 0.02, 0.1), 
                         vec3(0.2, 0.1, 0.4) * gridPulse, 
                         gridIntensity * timePulse);

    // Rain with cyberpunk glow (brighter and more colorful)
    vec2 gv = floor(uv * cColumns);
    float rnd = h11(gv.x + iTime * 0.1) + 0.1;
    float bw = 1.5 - fract((gv.y * 0.0028) + iTime * rnd * 1.8);

    // FFT-based brightness with stronger audio reactivity
    float fft = texture(iChannel0, vec2((1.0 - abs(uv.x)) * 0.8, 0.0)).x;
    fft -= abs(uv.x) * 0.3;

    // Amplify with audio - more aggressive response
    bw *= 1.0 + fft * 0.8 * cRainHighlights;
    bw += bw * clamp(pow(fft * 1.8 * cRainHighlights, 2.0) - 6.0, 0.0, 1.5);
    bw += bw * clamp(pow(fft * 1.4 * cRainHighlights, 3.0) - 12.0, 0.0, 1.0);
    bw = min(bw, 3.0);

    // Cyberpunk color palette (cyan, magenta, purple, orange)
    float colorPhase = gv.x * 0.2 + iTime * 0.3;
    vec3 cyberColor = vec3(
        sin(colorPhase) * 0.4 + 0.6,          // Red (cyan base)
        sin(colorPhase + 2.0) * 0.4 + 0.8,     // Green (bright)
        sin(colorPhase + 4.0) * 0.4 + 0.6      // Blue (magenta base)
    );
    
    // Add purple/orange highlights based on position
    if (fract(gv.x * 0.5) < 0.1 || fract(gv.y * 0.5) < 0.1)
    {
        cyberColor = mix(cyberColor, vec3(0.8, 0.2, 0.8), 0.5); // Purple
    }
    else if (fract(gv.x * 0.3 + iTime * 0.2) < 0.05)
    {
        cyberColor = mix(cyberColor, vec3(1.0, 0.5, 0.2), 0.4); // Orange
    }

    // Combine with grid
    vec3 col = mix(gridColor, cyberColor, bw * 0.9);

    // Add scan lines (cyberpunk monitor effect)
    float scanLine = sin(uv.y * cResolution.y * 2.0 + iTime * 5.0) * 0.1 + 0.9;
    col *= scanLine;

    // Add glitch effect (occasional horizontal distortions)
    if (fract(iTime * 0.8 + gv.x * 0.1) < 0.02)
    {
        float glitchOffset = sin(iTime * 10.0 + gv.x) * 0.1;
        float glitch = texture(iChannel0, vec2(uv.x + glitchOffset, uv.y)).x;
        col = mix(col, vec3(glitch * 0.5, glitch * 0.8, glitch), 0.3);
    }

    // Add vignette to focus the effect
    float vignette = length(uv) * cVIGNETTEINTENSITY * 1.2;
    col -= vignette * 0.3;

    // Ensure brightness minimum for cyberpunk look
    col = max(col, vec3(0.05, 0.02, 0.08));

    FragColor = vec4(col, 1.0);
}
