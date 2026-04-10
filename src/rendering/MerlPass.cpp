#include "MerlPass.hpp"
#include "geometry.hpp"
#include <memory>
#include <cstdio>
#include <vector>

#define BRDF_SAMPLING_RES_THETA_H 90
#define BRDF_SAMPLING_RES_THETA_D 90
#define BRDF_SAMPLING_RES_PHI_D   360

#define RED_SCALE   (1.0 / 1500.0)
#define GREEN_SCALE (1.15 / 1500.0)
#define BLUE_SCALE  (1.66 / 1500.0)

bool MerlPass::loadMerlBRDF(const std::string& filename) {
    FILE* f = fopen(filename.c_str(), "rb");
    if (!f) {
        fprintf(stderr, "Failed to open BRDF file: %s\n", filename.c_str());
        return false;
    }

    int dims[3];
    fread(dims, sizeof(int), 3, f);
    int n = dims[0] * dims[1] * dims[2];

    if (n != BRDF_SAMPLING_RES_THETA_H *
             BRDF_SAMPLING_RES_THETA_D *
             BRDF_SAMPLING_RES_PHI_D / 2) {
        fprintf(stderr, "MERL BRDF dimensions mismatch\n");
        fclose(f);
        return false;
    }

    std::vector<double> brdf(3 * n);
    fread(brdf.data(), sizeof(double), 3 * n, f);
    fclose(f);

    int phiHalf = BRDF_SAMPLING_RES_PHI_D / 2;
    int thetaD = BRDF_SAMPLING_RES_THETA_D;
    int thetaH = BRDF_SAMPLING_RES_THETA_H;

    std::vector<float> texData(phiHalf * thetaD * thetaH * 3);

    for (int ith = 0; ith < thetaH; ith++) {
        for (int itd = 0; itd < thetaD; itd++) {
            for (int ipd = 0; ipd < phiHalf; ipd++) {
                int merlIdx = ipd + itd * phiHalf + ith * phiHalf * thetaD;

                int texX = ipd;
                int texY = itd;
                int texZ = ith;
                int texIdx = (texZ * thetaD * phiHalf + texY * phiHalf + texX) * 3;

                texData[texIdx + 0] = static_cast<float>(brdf[merlIdx] * RED_SCALE);
                texData[texIdx + 1] = static_cast<float>(brdf[merlIdx + n] * GREEN_SCALE);
                texData[texIdx + 2] = static_cast<float>(brdf[merlIdx + 2 * n] * BLUE_SCALE);
            }
        }
    }

    glGenTextures(1, &brdfTexture);
    glBindTexture(GL_TEXTURE_3D, brdfTexture);
    glTexImage3D(GL_TEXTURE_3D, 0, GL_RGB32F,
                 phiHalf, thetaD, thetaH,
                 0, GL_RGB, GL_FLOAT, texData.data());
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_3D, 0);

    return true;
}

void MerlPass::init() {
    pass_name = "Merl pass";

    v_mesh.emplace_back(std::make_unique<Mesh>());
    auto loader = std::make_unique<objLoader>();
    loader->parse("assets\\bunny_10k.obj", *(v_mesh.back()));
    if (!v_mesh.back()->set_buffer()) {
        exit(-1);
    }

    parser = std::make_unique<ShaderParser>("src\\shader\\merl.vert", "src\\shader\\merl.frag");

    camera = std::make_unique<Camera>();

    lightPos = glm::vec3(2.0f, 3.0f, 1.0f);
    lightRotationAngle = 0.0f;
    lightRotationRadius = 3.0f;
    lightRotationSpeed = 0.5f;

    if (!loadMerlBRDF("assets\\brdf\\chrome-steel.binary")) {
        fprintf(stderr, "Warning: BRDF not loaded, rendering will be incorrect\n");
    }
}

void MerlPass::execute() {

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    parser->use();

    camera->Rotate(0.0f, 0.01f);
    glm::mat4 view       = camera->GetViewMatrix();
    glm::mat4 projection = camera->GetProjectionMatrix(1.25f);

    parser->setProjectionMatrix(projection);
    parser->setViewMatrix(view);
    parser->setModelMatrix(glm::mat4(1.4f));

    lightRotationAngle += lightRotationSpeed * 0.016f;
    lightPos.x = glm::cos(lightRotationAngle) * lightRotationRadius;
    lightPos.z = glm::sin(lightRotationAngle) * lightRotationRadius;
    lightPos.y = 3.0f;

    parser->setVec3("lightPos", lightPos);
    parser->setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    parser->setFloat("lightIntensity", 1000.0f);
    parser->setVec3("viewPos", camera->GetPosition());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_3D, brdfTexture);
    parser->setInt("brdfLUT", 0);

    glBindVertexArray(v_mesh.back()->get_VAO());

    GLsizei index_count = v_mesh.back()->get_index_count();
    glDrawElements(GL_TRIANGLES, index_count, GL_UNSIGNED_INT, 0);

    glBindVertexArray(0);
}

void MerlPass::clean() {
    parser = nullptr;
    v_mesh.clear();
    if (brdfTexture) {
        glDeleteTextures(1, &brdfTexture);
        brdfTexture = 0;
    }
}

MerlPass::MerlPass() {
    init();
}
