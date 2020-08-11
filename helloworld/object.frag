#version 330 core
out vec4 FragColor;
in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;  
uniform vec3 viewPos;
uniform vec3 lightPos;
uniform sampler2D ourTexture;
void main()
{  
    vec3 lightAmbient = vec3(0.2, 0.2, 0.2);
	vec3 lightDiffuse = vec3(0.8, 0.8, 0.8);
	vec3 lightSpecular = vec3(1.0, 1.0, 1.0);

	vec3 objectColor = texture(ourTexture, vec2(TexCoords.x, 1-TexCoords.y)).rgb;
	vec3 spe = vec3(0.5f, 0.5f, 0.5f);
	float shininess = 32;
    // ambient
	vec3 ambient = lightAmbient * objectColor;

	// diffuse 
	vec3 norm = normalize(Normal);
	vec3 lightDir = normalize(lightPos - FragPos);
	float diff = max(dot(norm, lightDir), 0.0);
	vec3 diffuse = lightDiffuse * (diff * objectColor);

	// specular
	vec3 viewDir = normalize(viewPos - FragPos);
	vec3 reflectDir = reflect(-lightDir, norm);
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), shininess);
	vec3 specular = lightSpecular * (spec * objectColor);

	vec3 result = ambient + diffuse + specular;
	FragColor = vec4(result, 1.0);
	//FragColor = texture(ourTexture, vec2(TexCoords.x, 1-TexCoords.y));
} 