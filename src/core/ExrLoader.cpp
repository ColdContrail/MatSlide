#define TINYEXR_IMPLEMENTATION
#include "ExrLoader.hpp"
#include <tinyexr.h>
#include <glm/glm.hpp>
#include <glm/common.hpp>
#include <iostream>
#include <cmath>

ExrLoader::ExrLoader()
    : width(0), height(0), textureID(0), cubemapID(0), loaded(false) {}

ExrLoader::~ExrLoader() {
    clean();
}
glm::vec3 ExrLoader::getPixel(int x, int y) {
    x = x % width;
    if (x < 0) x += width;

    y = glm::clamp(y, 0, height - 1);

    size_t idx = static_cast<size_t>(y) * width + x;
    return glm::vec3(
        pixels[idx * 3 + 0],
        pixels[idx * 3 + 1],
        pixels[idx * 3 + 2]
    );
}

bool ExrLoader::load(const std::string& filepath) {
    float* out_rgba = nullptr;
    const char* err = nullptr;

    int ret = LoadEXR(&out_rgba, &width, &height, filepath.c_str(), &err);
    if (ret != TINYEXR_SUCCESS) {
        std::cerr << "Failed to load EXR: " << filepath << std::endl;
        if (err) {
            std::cerr << "  Error: " << err << std::endl;
            FreeEXRErrorMessage(err);
        }
        return false;
    }

    pixels.resize(static_cast<size_t>(width) * height * 3);
    for (size_t i = 0; i < static_cast<size_t>(width) * height; ++i) {
        pixels[i * 3 + 0] = out_rgba[i * 4 + 0];
        pixels[i * 3 + 1] = out_rgba[i * 4 + 1];
        pixels[i * 3 + 2] = out_rgba[i * 4 + 2];
    }

    free(out_rgba);
    loaded = true;
    return true;
}

void ExrLoader::equirectToCubemapFace(int face, int size, std::vector<float>& faceData) {
    faceData.resize(static_cast<size_t>(size) * size * 3);
    const float PI = 3.14159265359f;

    for (int y = 0; y < size; ++y) {
        for (int x = 0; x < size; ++x) {
            float s = 2.0f * (static_cast<float>(x) + 0.5f) / size - 1.0f;
            float t = 2.0f * (static_cast<float>(y) + 0.5f) / size - 1.0f;

            glm::vec3 dir;
            switch (face) {
                case 0: dir = glm::vec3( 1.0f,   -t,   -s); break; 
                case 1: dir = glm::vec3(-1.0f,   -t,    s); break; 
                case 2: dir = glm::vec3(    s, 1.0f,    t); break; 
                case 3: dir = glm::vec3(    s,-1.0f,   -t); break; 
                case 4: dir = glm::vec3(    s,   -t, 1.0f); break; 
                case 5: dir = glm::vec3(   -s,   -t,-1.0f); break; 
            }
            dir = glm::normalize(dir);
            dir = glm::normalize(dir);

            float phi = std::atan2(dir.z, dir.x);
            if (phi < 0.0f) phi += 2.0f * PI;
            float theta = std::acos(glm::clamp(dir.y, -1.0f, 1.0f));

            float srcUf = phi / (2.0f * PI) * static_cast<float>(width);
            float srcVf = theta / PI * static_cast<float>(height);

            int x0 = static_cast<int>(std::floor(srcUf - 0.5f));
            int y0 = static_cast<int>(std::floor(srcVf - 0.5f));
            int x1 = x0 + 1;
            int y1 = y0 + 1;

            float tx = (srcUf - 0.5f) - static_cast<float>(x0);
            float ty = (srcVf - 0.5f) - static_cast<float>(y0);
            tx = glm::clamp(tx, 0.0f, 1.0f);
            ty = glm::clamp(ty, 0.0f, 1.0f);

            glm::vec3 c00 = getPixel(x0, y0);
            glm::vec3 c10 = getPixel(x1, y0);
            glm::vec3 c01 = getPixel(x0, y1);
            glm::vec3 c11 = getPixel(x1, y1);

            glm::vec3 c0 = glm::mix(c00, c10, tx);
            glm::vec3 c1 = glm::mix(c01, c11, tx);

            glm::vec3 finalColor = glm::mix(c0, c1, ty);

            size_t dstIdx = static_cast<size_t>(y) * size + x;
            faceData[dstIdx * 3 + 0] = finalColor.r;
            faceData[dstIdx * 3 + 1] = finalColor.g;
            faceData[dstIdx * 3 + 2] = finalColor.b;
        }
    }
}

void ExrLoader::uploadCubemap(int cubeSize) {
    if (!loaded || pixels.empty()) {
        std::cerr << "ExrLoader: no data to upload cubemap" << std::endl;
        return;
    }

    if (cubemapID) {
        glDeleteTextures(1, &cubemapID);
        cubemapID = 0;
    }

    glCreateTextures(GL_TEXTURE_CUBE_MAP, 1, &cubemapID);
    glTextureStorage2D(cubemapID, 1, GL_RGB32F, cubeSize, cubeSize);

    for (int face = 0; face < 6; ++face) {
        std::vector<float> faceData;
        equirectToCubemapFace(face, cubeSize, faceData);
        glTextureSubImage3D(cubemapID, 0, 0, 0, face,
                            cubeSize, cubeSize, 1,
                            GL_RGB, GL_FLOAT, faceData.data());
    }

    glTextureParameteri(cubemapID, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTextureParameteri(cubemapID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTextureParameteri(cubemapID, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTextureParameteri(cubemapID, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTextureParameteri(cubemapID, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void ExrLoader::uploadTexture() {
    if (!loaded || pixels.empty()) {
        std::cerr << "ExrLoader: no data to upload" << std::endl;
        return;
    }

    if (textureID) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }

    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB32F,
                 width, height, 0,
                 GL_RGB, GL_FLOAT, pixels.data());
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
}

void ExrLoader::clean() {
    pixels.clear();
    pixels.shrink_to_fit();
    if (textureID) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
    if (cubemapID) {
        glDeleteTextures(1, &cubemapID);
        cubemapID = 0;
    }
    width = 0;
    height = 0;
    loaded = false;
}

GLuint ExrLoader::getTextureID() const {
    return textureID;
}

GLuint ExrLoader::getCubemapID() const {
    return cubemapID;
}

int ExrLoader::getWidth() const {
    return width;
}

int ExrLoader::getHeight() const {
    return height;
}

bool ExrLoader::isLoaded() const {
    return loaded;
}
