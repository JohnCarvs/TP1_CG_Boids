#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 aTex;
layout (location = 3) in vec3 aNormal;

out vec3 color;
out vec2 texCoord;
out vec3 Normal;
out vec3 FragPos;

uniform float time;
uniform mat4 camMatrix;
uniform mat4 model;

void main()
{
   // operações para renderizar em 3d
   FragPos = vec3(model * vec4(aPos, 1.0));
   gl_Position = camMatrix * vec4(FragPos, 1.0);
   
   // Transformar normal para world space
   Normal = mat3(transpose(inverse(model))) * aNormal;

   color = aColor;
   texCoord = aTex;
}