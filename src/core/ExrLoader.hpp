#pragma once
#include <glad/glad.h>
#include <string>
#include <vector>
#include <glm/glm.hpp>

class ExrLoader {
public:
    ExrLoader();
    ~ExrLoader();

    bool load(const std::string& filepath);
    void uploadTexture();
    void uploadCubemap(int cubeSize = 512);
    void clean();

    GLuint getTextureID() const;
    GLuint getCubemapID() const;
    int getWidth() const;
    int getHeight() const;
    bool isLoaded() const;

private:
    std::vector<float> pixels;
    int width;
    int height;
    GLuint textureID;
    GLuint cubemapID;
    bool loaded;

    void equirectToCubemapFace(int face, int size, std::vector<float>& faceData);
    glm::vec3 getPixel(int x, int y);
};
