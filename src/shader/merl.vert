#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
// 如果有切线，可以在此处加入 location = 2

out vec3 WorldPos;
out vec3 Normal;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    // 计算世界空间位置
    WorldPos = vec3(model * vec4(aPos, 1.0));
    
    // 计算法线矩阵 (处理非均匀缩放)
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    Normal = normalize(normalMatrix * aNormal);

    gl_Position = projection * view * vec4(WorldPos, 1.0);
}