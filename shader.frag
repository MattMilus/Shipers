#version 330 core

uniform vec2 uResolution;
uniform int uCount;
uniform float uTime;
uniform vec2 uBoatPosition;

uniform vec2 uPositions[256];

uniform float uLifetimes[256];

uniform vec3 uColors[256];


struct SDF {
    float dist;
    vec4 col;
};

float sdCircle(vec2 p, vec2 center, float r) {
    return length(p - center) - r;
}

//p - Point tested, b - half dimensions, r - corner radii, 
float sdRoundBox(vec2 p, vec2 b, vec4 r) {
    r.xy = (p.x > 0.0) ? r.xy : r.zw;
    r.x  = (p.y > 0.0) ? r.x  : r.y;
    vec2 q = abs(p) - b + r.x;
    return min(max(q.x, q.y), 0.0) + length(max(q, 0.0)) - r.x;
}

SDF smoothUnion(SDF a, SDF b, float smoothness) {
    float ratio = 0.5;
    float h = clamp(ratio + (1 - ratio) * (b.dist - a.dist) / smoothness, 0.0, 1.0);
    float dist = mix(b.dist, a.dist, h) - smoothness * h * (1.0 - h);

    // Weight color contribution by each shape's alpha
    float alphaA = a.col.a;
    float alphaB = b.col.a;
    float totalAlpha = alphaA + alphaB;

    vec4 col;
    if (totalAlpha < 0.0001) {
        col = vec4(0.0);
    } else {
        col.rgb = (a.col.rgb * alphaA + b.col.rgb * alphaB) / totalAlpha;
        col.a = mix(b.col.a, a.col.a, h);
    }

    return SDF(dist, col);
}



SDF scene(vec2 p) {
    float smoothness = 0.08;
    const float kMaxDistance = 1e5;
    SDF result = SDF(kMaxDistance, vec4(0.0));

    for (int i = 0; i < uCount; ++i) {
        SDF circle = SDF(
            abs(sdCircle(p, uPositions[i], uLifetimes[i] * 0.1)) - 0.001,
            vec4(
                uColors[i].rgb,
                clamp(1.0 - uLifetimes[i], 0.0, 1.0)
            )
        );

        result = smoothUnion(result, circle, smoothness);
    }

    return result;
}

void main() {
    vec2 uv = gl_FragCoord.xy / uResolution;
    SDF result = scene(uv);
    
    vec3 backgroundColor = vec3(0.2, 0.3, 0.7);

    // Narrow band = foam/edge, wide band = filled interior
    float edgeWidth = 0.0125;
    float foamAlpha  = smoothstep(edgeWidth, 0.0, abs(result.dist)) * result.col.a;
    float innerAlpha = smoothstep(0.02, -0.02, result.dist) * result.col.a;

    // Interior is background color, edge is true circle color
    vec3 finalColor = backgroundColor;
    finalColor = mix(finalColor, backgroundColor, innerAlpha);  // hollow interior = bg
    finalColor = mix(finalColor, result.col.rgb, foamAlpha);    // foam edge = true color
    
    gl_FragColor = vec4(finalColor, 1.0);
}