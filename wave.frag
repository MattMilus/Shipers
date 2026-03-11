#version 330 core

out vec4 colour;

uniform sampler2D image;

uniform vec2 uResolution;
uniform float uTime;

uniform vec2 uPathHistory[256];

void main() {
    vec2 pos = gl_FragCoord.xy / uResolution;
    pos.y = 1.0 - pos.y; // visual fix
    
    float x = sin(pos.y * 3.78 + uTime) * 0.05;
    float y = cos(pos.x * 3.78 + uTime) * 0.05;
    vec2 totalOffset = vec2(x, y);

    float waveStrength = 0.03;
    float speed = 0.3;

    for (int i = 0; i < 256; ++i) {
        vec2 centre = uPathHistory[i];
        vec2 dir = pos - centre;
        float dist = length(dir);
        float age = float(i) / 255.0;

        float d = dist - age * speed;
        float wave = d * (1.0 - smoothstep(0.0, waveStrength, abs(d)));
        float strength = 1.0 - age;

        dir = normalize(dir);
        totalOffset += dir * wave * strength;
    }

    colour = texture(image, pos + totalOffset);
}