#version 330 core

in vec4 fragPosLightSpace;

uniform sampler2D shadowMap;

out vec4 fragment_color;

float CalculateShadow() {
    // Transform coordinates to normalized device coordinates.
    vec3 projectedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

    // Transform coordinates from [-1, 1] to [0, 1].
    projectedCoords = projectedCoords * 0.5 + 0.5;

    // Ignore fragments outside the shadow map.
    if (projectedCoords.x < 0.0 || projectedCoords.x > 1.0 ||
        projectedCoords.y < 0.0 || projectedCoords.y > 1.0 ||
        projectedCoords.z < 0.0 || projectedCoords.z > 1.0) {
        return 0.0;
    }

    // Get the closest depth from the light's point of view.
    float closestDepth = texture(shadowMap, projectedCoords.xy).r;

    float currentDepth = projectedCoords.z;

    float bias = 0.005;

    if (currentDepth - bias > closestDepth) {
        return 1.0;
    }

    return 0.0;
}

void main() {
    vec3 baseColor = vec3(1.0, 0.5, 0.2);

    float shadow = CalculateShadow();
    float illumination = 1.0 - shadow * 0.7;

    fragment_color = vec4(baseColor * illumination, 1.0);
}
