#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoords;
layout (location = 2) in vec3 aNormal;
out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;
uniform mat4 mvp;
void main()
{
    FragPos = aPos;
    Normal = aNormal;  
    TexCoords = aTexCoords;   
    gl_Position = mvp*vec4(aPos, 1.0);
}