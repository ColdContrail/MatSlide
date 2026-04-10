#pragma once

struct MaterialParameters {
    float ggxRoughness = 0.051f;

    float diffuseR = 0.086f;
    float diffuseG = 0.060f;
    float diffuseB = 0.042f;

    float specularR = 0.04f;
    float specularG = 0.0357f;
    float specularB = 0.026f;

    float optimalThresholdR = 0.048f;
    float optimalThresholdG = 0.028f;
    float optimalThresholdB = 0.018f;

    float* data() { return &ggxRoughness; }
    const float* data() const { return &ggxRoughness; }
    int size() const { return 10; }
};
