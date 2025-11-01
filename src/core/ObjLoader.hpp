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
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;

    bool parse(const std::string& filename, Mesh& mesh) {

        std::ifstream file(filename);
        if (!file) {
            std::cout << "Failed to read file" << std::endl;
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
                    vertices.push_back(v);
                }
            } else if (keyword == "vn"){
                glm::vec3 v;
                if (ss >> v.x >> v.y >> v.z) {
                    normals.push_back(v);
                }
            } else if (keyword == "f") {
                std::string faceData;
                unsigned int indices[2];
                while (ss >> faceData) {
                    std::stringstream faceSS(faceData);
                    std::string indexStr;
                    if (std::getline(faceSS, indexStr, '/')) {
                        indices[0] = std::stoi(indexStr) - 1;
                    }
                    if (std::getline(faceSS, indexStr, '/')) {
                        ;
                        // int vertexTexIndex = std::stoi(indexStr) - 1;
                    }
                    if (std::getline(faceSS, indexStr, '/')) {
                        indices[1] = std::stoi(indexStr) - 1;
                    }

                    Vertex v;
                    v.position = vertices[indices[0]];
                    v.normal = normals[indices[1]];
                    v.texCoords = glm::vec2(0.0f, 0.0f);
                    mesh.push_vertex(std::move(v));
                    mesh.push_index();
                }
            }
        }
        file.close();
        return true;
    }
};
