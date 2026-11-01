#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec3 aColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrix;

out vec4 fragPosLightSpace;
out vec3 fragNormal;
out vec3 fragColor;

void main() {
  vec4 worldPosition = model * vec4(aPosition, 1.0);

  gl_Position = projection * view * worldPosition;

  fragPosLightSpace = lightSpaceMatrix * worldPosition;

  mat3 normalMatrix = mat3(transpose(inverse(model)));
  fragNormal = normalMatrix * aNormal;
  fragColor = aColor;
}
