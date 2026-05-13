#version 330 core
out vec4 colour;
uniform sampler2D image;
uniform vec2 uResolution;
uniform float uTime;
uniform vec2 uPathHistory[1024];
uniform int uActivePathSamples;
uniform int uCameraHistoryOffset;

const int BOAT_HISTORY_SIZE = 256;
const int TOTAL_HISTORY_SIZE = 1024;

float speed = 0.3;
float waveStrength = 0.03;

float getOffset(float age, float dist) {
    float d = dist - age * speed;
    d = d * (1.0 - smoothstep(0.0, waveStrength, abs(d)));
    //d *= smoothstep(0.0, 0.15, age);        //intro
    return d;
}

void main() {
    vec2 pos = gl_FragCoord.xy / uResolution;
    pos.y = 1.0 - pos.y; // visual fix
    vec2 camera = uPathHistory[uCameraHistoryOffset] - vec2(0.5);
    pos += camera;

    float x = sin(pos.y * 6.78 + uTime) * 0.05;
    float y = cos(pos.x * 3.78 + uTime) * 0.05;
    vec2 totalDir = vec2(0, 0);
    vec2 ambientWavesDir = vec2(x, y);

    vec3 totalOffsets = vec3(0.0);

    for (int i = 0; i < TOTAL_HISTORY_SIZE; ++i) {
        if (i >= uActivePathSamples) {
            break;
        }

        vec2 centre = uPathHistory[i];
        vec2 dir = pos - centre;
        float dist = length(dir);
        float age = float(i % BOAT_HISTORY_SIZE) / float(BOAT_HISTORY_SIZE - 1);
        float wave = getOffset(age, dist);
        float strength = 1.0 - age;
        if (dist > 0.0001) {
            dir = normalize(dir);
        } else {
            dir = vec2(0.0, 0.0);
        }
        totalDir += dir * wave * strength;

        totalOffsets.r += getOffset(age, dist * 0.98) * strength;
        totalOffsets.g += getOffset(age, dist)        * strength;
        totalOffsets.b += getOffset(age, dist * 1.02) * strength;
    }

    // pseudo piana do poprawy
    float r = texture(image, fract(pos + ambientWavesDir + totalDir * (0.5 + totalOffsets.r))).r + abs(totalDir.x) * 1.2;
    float g = texture(image, fract(pos + ambientWavesDir + totalDir * (0.5 + totalOffsets.g))).g + abs(totalDir.x) * 1.2;
    float b = texture(image, fract(pos + ambientWavesDir + totalDir * (0.25 + totalOffsets.b))).b + abs(totalDir.x) * 1.0;

    colour = vec4(r, g, b, 1.0);
}
