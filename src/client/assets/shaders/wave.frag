#version 330 core

out vec4 colour;

uniform sampler2D image; 
uniform vec2 uResolution;
uniform float uTime;

// --- Ripple Uniforms ---
uniform vec2 uPathHistory[512];
uniform int uPlayerId;

// --- FBM Water Uniforms ---
uniform float mulscale = 5.0;
uniform float height = 0.6;
uniform float tide = 0.1;
uniform float foamthickness = 0.1;
uniform float timescale = 1.0;

// Alpha channels lowered to reveal the background image
uniform vec4 WATER_COL = vec4(0.04, 0.38, 0.88, 0.6);
uniform vec4 WATER2_COL = vec4(0.04, 0.35, 0.78, 0.8);
uniform vec4 FOAM_COL = vec4(0.8125, 0.9609, 0.9648, 1.0);

const int OCTAVE = 6;
float speed = 0.3;
float waveStrength = 0.03;

// --- Noise Functions ---
float rand(vec2 coordIn) {
    return fract(sin(dot(coordIn, vec2(23.53, 44.0))) * 42350.45);
}

float perlin(vec2 coordIn) {
    vec2 i = floor(coordIn);
    vec2 j = fract(coordIn);
    vec2 coord = smoothstep(0.0, 1.0, j);
    
    float a = rand(i);
    float b = rand(i + vec2(1.0, 0.0));
    float c = rand(i + vec2(0.0, 1.0));
    float d = rand(i + vec2(1.0, 1.0));

    return mix(mix(a, b, coord.x), mix(c, d, coord.x), coord.y);
}

float fbm(vec2 coordIn) {
    float value = 0.0;
    float scale = 0.5;
    
    for(int i = 0; i < OCTAVE; i++) {
        value += perlin(coordIn) * scale;
        coordIn *= 2.0;
        scale *= 0.5;
    }
    return value;
}

// --- Ripple Offset Function ---
float getOffset(float age, float dist) {
    float d = dist - age * speed;
    d = d * (1.0 - smoothstep(0.0, waveStrength, abs(d)));
    return d;
}

void main() {
    vec2 pos = gl_FragCoord.xy / uResolution;
    pos.y = 1.0 - pos.y; // visual fix
    
    vec2 camera = uPathHistory[0] - vec2(0.5);
    pos += camera;

    // 1. Calculate Distortions
    float x = sin(pos.y * 6.78 + uTime) * 0.05;
    float y = cos(pos.x * 3.78 + uTime) * 0.05;
    vec2 ambientWavesDir = vec2(x, y);

    vec2 totalDir = vec2(0.0);
    vec3 totalOffsets = vec3(0.0);

    for (int i = 0; i < 512; ++i) {
        vec2 centre = uPathHistory[i];
        vec2 dir = pos - centre;
        float dist = length(dir);
        float age = 0.25;
        if(i < 256) age = float(i) / 255.0;
        
        float wave = getOffset(age, dist);
        float strength = 1.0 - age;
        
        if (dist > 0.0) {
            dir = normalize(dir);
            totalDir += dir * wave * strength;
            
            // Reintroduce chromatic aberration offsets
            totalOffsets.r += getOffset(age, dist * 0.98) * strength;
            totalOffsets.g += getOffset(age, dist)        * strength;
            totalOffsets.b += getOffset(age, dist * 1.02) * strength;
        }
    }

    vec2 baseUV = pos + ambientWavesDir;

    // 2. Sample the background image with chromatic aberration
    // Fract is kept here so the image itself tiles properly
    float r = texture(image, fract(baseUV + totalDir * (0.5 + totalOffsets.r))).r;
    float g = texture(image, fract(baseUV + totalDir * (0.5 + totalOffsets.g))).g;
    float b = texture(image, fract(baseUV + totalDir * (0.25 + totalOffsets.b))).b;
    vec3 bgImageCol = vec3(r, g, b);
    
    // 3. Generate Procedural Water
    // Fract is removed to prevent borders/seams in the noise generation
    vec2 fbmUV = baseUV + totalDir * 0.5;
    float newtime = uTime * timescale;
    
    float fbmval = fbm(vec2(fbmUV.x * mulscale + 0.2 * sin(0.3 * newtime) + 0.15 * newtime, 
                            -0.05 * newtime + fbmUV.y * mulscale + 0.1 * cos(0.68 * newtime)));
                            
    float fbmvalshadow = fbm(vec2(fbmUV.x * mulscale + 0.2 * sin(-0.6 * newtime + 25.0 * fbmUV.y) + 0.15 * newtime + 3.0, 
                                  -0.05 * newtime + fbmUV.y * mulscale + 0.13 * cos(-0.68 * newtime)) - 7.0 + 0.1 * sin(0.43 * newtime));
                                  
    float myheight = height + tide * sin(newtime + 5.0 * fbmUV.x - 8.0 * fbmUV.y);
    float shadowheight = height + tide * 1.3 * cos(newtime + 2.0 * fbmUV.x - 2.0 * fbmUV.y);
    
    float withinFoam = step(myheight, fbmval) * step(fbmval, myheight + foamthickness);
    float shadow = (1.0 - withinFoam) * step(shadowheight, fbmvalshadow) * step(fbmvalshadow, shadowheight + foamthickness * 0.7);
    
    vec4 waterFinalCol = withinFoam * FOAM_COL + shadow * WATER2_COL + ((1.0 - withinFoam) * (1.0 - shadow)) * WATER_COL;
    
    // 4. Add the pseudo piana (ripple foam)
    float pseudoPiana = abs(totalDir.x) * 1.2;
    waterFinalCol.rgb += vec3(pseudoPiana);

    // 5. Blend background image with water
    vec3 finalCol = mix(bgImageCol, waterFinalCol.rgb, waterFinalCol.a);

    colour = vec4(finalCol, 1.0);
}