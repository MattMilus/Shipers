#version 330 core

out vec4 colour;

uniform vec2 uResolution;

void main() {
    vec2 uv = gl_FragCoord.xy / uResolution;
    vec2 grid = floor(uv * 20.0);
    float checker = mod(grid.x + grid.y, 2.0);
    vec3 col = mix(vec3(0.0, 0.545, 0.545), vec3(0.0, 0.878, 0.878), checker);
    colour = vec4(col, 1.0);
}