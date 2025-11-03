#version 330 core
out vec4 FragColor;

in vec3 color;

in vec2 texCoord;

uniform sampler2D tex0;

uniform float time;

void main()
{
   //FragColor = vec4(color+0.5+ 0.5*sin(10*time), 1.0f);
   FragColor = vec4(color, 1.0f);
   //FragColor = texture(tex0, texCoord);
}