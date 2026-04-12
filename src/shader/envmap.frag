#version 450 core

in vec3 WorldPos;

out vec4 FragColor;

uniform samplerCube envMap;

void main()
{
    vec3 color = texture(envMap, WorldPos).rgb;

    color = color / (color + vec3(1.0));
    color = pow(color, vec3(1.0 / 2.2));

    FragColor = vec4(color, 1.0);
}
