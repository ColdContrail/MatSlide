#pragma once
#include <glm/glm.hpp>
#include <glad/glad.h>
#include <iostream>
#include <iomanip>
#include <vector>

struct Vertex {
public:
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texCoords;
};

class Mesh {
private:
    std::vector<Vertex> vertices;
    std::vector<GLuint> indices;
    GLuint VAO, VBO, EBO;
    bool buffersInitialized = false;
    int index = 0;

    void printMeshData() const {
        std::cout << "=== Mesh Data ===" << std::endl;
        
        std::cout << "Vertices (" << vertices.size() << "):" << std::endl;
        for (size_t i = 0; i < vertices.size(); ++i) {
            const Vertex& v = vertices[i];
            std::cout << "  Vertex " << i << ":" << std::endl;
            std::cout << "    Position:  (" 
                      << std::fixed << std::setprecision(2)
                      << v.position.x << ", " << v.position.y << ", " << v.position.z << ")" << std::endl;
            std::cout << "    Normal:    (" 
                      << v.normal.x << ", " << v.normal.y << ", " << v.normal.z << ")" << std::endl;
            std::cout << "    TexCoords: (" 
                      << v.texCoords.x << ", " << v.texCoords.y << ")" << std::endl;
        }
        
        std::cout << "Indices (" << indices.size() << "):" << std::endl;
        std::cout << "  ";
        for (size_t i = 0; i < indices.size(); ++i) {
            std::cout << indices[i];
            if (i < indices.size() - 1) {
                std::cout << ", ";
            }
        }
        std::cout << std::endl;
    }

    void delete_buffer() {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        VAO = 0; VBO = 0; EBO = 0;
    }
public:
    Mesh() = default;
    ~Mesh() {
        delete_buffer();
    }

    Mesh(const Mesh&) = delete;
    Mesh& operator=(const Mesh&) = delete;

    Mesh& operator=(Mesh&& other) noexcept {
        if (this != &other) {
            delete_buffer();
            
            vertices = std::move(other.vertices);
            indices = std::move(other.indices);
            VAO = other.VAO;
            VBO = other.VBO;
            EBO = other.EBO;
            buffersInitialized = other.buffersInitialized;
            
            other.VAO = other.VBO = other.EBO = 0;
            other.buffersInitialized = false;
        }
        return *this;
    }

    bool set_buffer() {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), 
                     vertices.data(), GL_STATIC_DRAW);
        
        

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(GLuint),
                     indices.data(), GL_STATIC_DRAW);
        // std::cout << indices.size() << std::endl;
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                             (void*)0);
        glEnableVertexAttribArray(0);
        
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), 
                             (void*)offsetof(Vertex, normal));
        glEnableVertexAttribArray(1);
        
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                             (void*)offsetof(Vertex, texCoords));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
        buffersInitialized = true;
        // printMeshData();
        return buffersInitialized;
    }

    void push_vertex(Vertex const& v) {
        vertices.emplace_back(v);
    }
    void push_vertex(Vertex&& v) {
        vertices.emplace_back(std::move(v));
    }
    void push_index(GLuint const& i) {
        indices.emplace_back(i);
    }
    void push_index(void) {
        indices.emplace_back(index);
        ++index;
    }

    GLsizei get_vertex_count() {
        return vertices.size();
    }
    GLsizei get_index_count() {
        return indices.size();
    }
    GLuint get_VAO() {
        return VAO;
    }
};