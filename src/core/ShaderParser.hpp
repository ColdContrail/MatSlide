#pragma once

#include "glm/fwd.hpp"
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <fstream>
#include <string>
#include <sstream>
#include <iostream>

class ShaderParser {
private:
    std::string readShaderFile(const char* filePath) {
        std::ifstream file;
        std::stringstream stream;
        
        file.exceptions(std::ifstream::failbit | std::ifstream::badbit);
        
        try {
            file.open(filePath);
            stream << file.rdbuf();
            file.close();
            return stream.str();
        } catch (std::ifstream::failure& e) {
            std::cout << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << filePath << std::endl;
            return "";
        }
    }
public:
    GLuint program;

    ShaderParser(std::string const& vertex_shader, std::string const& frag_shader) {
        std::string vertexCode = readShaderFile(vertex_shader.c_str());
        std::string fragmentCode = readShaderFile(frag_shader.c_str());
        
        const char* vShaderCode = vertexCode.c_str();
        const char* fShaderCode = fragmentCode.c_str();

        GLuint vertexshader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(vertexshader, 1, &vShaderCode, NULL);
        glCompileShader(vertexshader);

        GLuint fragshader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fragshader, 1, &fShaderCode, NULL);
        glCompileShader(fragshader);
        
        program = glCreateProgram();
        
        glAttachShader(program, vertexshader);
        glAttachShader(program, fragshader);
        glLinkProgram(program);
        
        glDeleteShader(vertexshader);
        glDeleteShader(fragshader);
    }
    void use() {
        glUseProgram(program);
    }

    void setMat4(const std::string& name, const glm::mat4& mat) const {
        glUniformMatrix4fv(glGetUniformLocation(program, name.c_str()), 1, GL_FALSE, glm::value_ptr(mat));
    }

    void setViewMatrix(const glm::mat4& view) const {
        setMat4("view", view);
    }

    void setProjectionMatrix(const glm::mat4& projection) const {
        setMat4("projection", projection);
    }

    void setModelMatrix(const glm::mat4& model) const {
        setMat4("model", model);
    }

    void setTransformMatrices(const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection) const {
        setModelMatrix(model);
        setViewMatrix(view);
        setProjectionMatrix(projection);
    }

    void setVec3(const std::string& name, const glm::vec3& vec) const {
        glUniform3fv(glGetUniformLocation(program, name.c_str()), 1, glm::value_ptr(vec));
    }

    void setFloat(const std::string& name, float value) const {
        glUniform1f(glGetUniformLocation(program, name.c_str()), value);
    }

    void setInt(const std::string& name, int value) const {
        glUniform1i(glGetUniformLocation(program, name.c_str()), value);
    }
};