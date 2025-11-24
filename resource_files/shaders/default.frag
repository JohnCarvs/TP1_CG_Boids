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

// Fog uniforms
uniform bool fogEnabled;
uniform vec3 fogColor;
uniform float fogDensity;
uniform float fogStart;
uniform float fogEnd;

void main()
{
   // Ambient Occlusion simples baseado na altura e orientação da normal
   float ao = 1.0;
   
   // AO baseado na altura (escurece objetos próximos ao chão)
   float heightFactor = smoothstep(-10.0, 10.0, FragPos.y);
   ao *= mix(0.4, 1.0, heightFactor);
   
   // AO baseado na normal (escurece faces viradas para baixo)
   vec3 normal = normalize(Normal);
   float normalAO = normal.y * 0.5 + 0.5; // Mapeia -1,1 para 0,1
   ao *= mix(0.6, 1.0, normalAO);
   
   // Ambient lighting com AO
   float ambient = 0.70f * ao;

   // Diffuse lighting (Lambertian)
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
   
   vec4 finalColor = vec4(color, 1.0f) * lightColor * lighting;
   //finalColor = texture(tex0, texCoord) * lightColor * lighting;
   
   // Aplicar fog se estiver ativado
   if (fogEnabled) {
       float distance = length(camPos - FragPos);
       
       // Fog linear
       float fogFactor = clamp((fogEnd - distance) / (fogEnd - fogStart), 0.0, 1.0);
       
       // Mix entre a cor final e a cor do fog
       FragColor = mix(vec4(fogColor, 1.0), finalColor, fogFactor);
   } else {
       FragColor = finalColor;
   }
}