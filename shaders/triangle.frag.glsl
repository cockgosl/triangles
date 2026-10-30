#version 330 core

in vec4 fragPosLightSpace;
in vec3 fragNormal;

uniform sampler2D shadowMap;
uniform vec3 lightDirection;
uniform float shadowBias;
uniform float shadowStrength;

out vec4 fragment_color;

const int kPcfRadius = 1;
const float kPcfSampleCount = 9.0;
const float kAmbientStrength = 0.2;

float CalculateShadow() {
    vec3 projectedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    projectedCoords = projectedCoords * 0.5 + 0.5;

    if (projectedCoords.x < 0.0 || projectedCoords.x > 1.0 ||
        projectedCoords.y < 0.0 || projectedCoords.y > 1.0 ||
        projectedCoords.z < 0.0 || projectedCoords.z > 1.0) {
        return 0.0;
    }

    float currentDepth = projectedCoords.z;

    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));

    float shadow = 0.0;

    for (int x = -kPcfRadius; x <= kPcfRadius; ++x) {
        for (int y = -kPcfRadius; y <= kPcfRadius; ++y) {
            vec2 sampleCoords = projectedCoords.xy + vec2(x, y) * texelSize;

            float closestDepth = texture(shadowMap, sampleCoords).r;

            if (currentDepth - shadowBias > closestDepth) {
                shadow += 1.0;
            }
        }
    }

    return shadow / kPcfSampleCount;
}

void main() {
    vec3 baseColor = vec3(1.0, 0.5, 0.2);

    vec3 normal = normalize(fragNormal);
    vec3 lightDir = normalize(lightDirection);

    // Handle triangles visible from their back side.
    if (!gl_FrontFacing) {
        normal = -normal;
    }

    // Lambert diffuse lighting.
    float diffuse = max(dot(normal, lightDir), 0.0);

    // Calculate shadow visibility.
    float shadow = CalculateShadow();
    float visibility = 1.0 - shadow * shadowStrength;

    // Combine ambient and diffuse lighting.
    float illumination =
        kAmbientStrength + (1.0 - kAmbientStrength) * diffuse * visibility;

    fragment_color = vec4(baseColor * illumination, 1.0);
}
