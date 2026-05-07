#pragma once
#include <cassert>
#include <ostream>
#include <sstream>
#include <vector>
#include <fstream>
#include <iostream>
#include <glm/glm.hpp>
#include "geometry.hpp"

class objLoader {
public:
    float scale;

    explicit objLoader(float s = 1.0f) : scale(s) {}

    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;

    bool parse(const std::string& filename, Mesh& mesh) {
        std::ifstream file(filename);
        if (!file) {
            std::cout << "Failed to read file: " << filename << std::endl;
            return false;
        }

        std::string line;
        while (std::getline(file, line)) {
            if (line.empty() || line[0] == '#') continue;

            std::stringstream ss(line);
            std::string keyword;
            ss >> keyword;

            if (keyword == "v") {
                glm::vec3 v;
                if (ss >> v.x >> v.y >> v.z) {
                    v *= scale;
                    vertices.push_back(v);
                }
            } else if (keyword == "vn") {
                glm::vec3 v;
                if (ss >> v.x >> v.y >> v.z) {
                    normals.push_back(glm::normalize(v));
                }
            } else if (keyword == "f") {
                std::string faceData;
                while (ss >> faceData) {
                    std::stringstream faceSS(faceData);
                    std::string indexStr;
                    
                    unsigned int vIndex = 0;
                    unsigned int vtIndex = 0;
                    unsigned int vnIndex = 0;

                    if (std::getline(faceSS, indexStr, '/')) {
                        if (!indexStr.empty()) vIndex = static_cast<unsigned int>(std::stoi(indexStr));
                    }
                    if (std::getline(faceSS, indexStr, '/')) {
                        if (!indexStr.empty()) vtIndex = static_cast<unsigned int>(std::stoi(indexStr));
                    }
                    if (std::getline(faceSS, indexStr, '/')) {
                        if (!indexStr.empty()) vnIndex = static_cast<unsigned int>(std::stoi(indexStr));
                    }

                    size_t vi = (vIndex > 0) ? (vIndex - 1) : 0;
                    size_t vni = (vnIndex > 0) ? (vnIndex - 1) : 0;

                    Vertex v;
                    
                    if (vi < vertices.size()) {
                        v.position = vertices[vi];
                    } else {
                        std::cerr << "Warning: vertex index out of bounds: " << vIndex << std::endl;
                        v.position = glm::vec3(0.0f);
                    }

                    if (vnIndex > 0 && vni < normals.size()) {
                        v.normal = normals[vni];
                    } else {
                        v.normal = glm::vec3(0.0f, 1.0f, 0.0f);
                    }

                    v.texCoords = glm::vec2(0.0f, 0.0f);
                    mesh.push_vertex(std::move(v));
                    mesh.push_index();
                }
            }
        }
        return true;
    }
};