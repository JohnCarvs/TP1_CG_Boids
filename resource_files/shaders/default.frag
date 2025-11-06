#version 330 core
out vec4 FragColor;

in vec3 color;
in vec2 texCoord;
in vec3 Normal;
in vec3 FragPos;

uniform sampler2D tex0;
uniform float time;
uniform vec4 lightColor;
uniform vec3 lightPos;
uniform vec3 camPos;

void main()
{
   // Ambient lighting
   float ambient = 0.70f;

   // Diffuse lighting (Lambertian)
   vec3 normal = normalize(Normal);
   vec3 lightDirection = normalize(lightPos - FragPos);
   float diffuse = max(dot(normal, lightDirection), 0.0f);

   // Specular lighting (brilho mais concentrado)
   float specularLight = 0.60f;
   vec3 viewDirection = normalize(camPos - FragPos);
   vec3 reflectionDirection = reflect(-lightDirection, normal);
   float specAmount = pow(max(dot(viewDirection, reflectionDirection), 0.0f), 32);
   float specular = specAmount * specularLight;

   // Combinar todos os componentes
   float lighting = ambient + diffuse + specular;
   
   FragColor = vec4(color, 1.0f) * lightColor * lighting;
   //FragColor = texture(tex0, texCoord) * lightColor * lighting;
}