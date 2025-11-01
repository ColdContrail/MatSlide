#include "RenderPass.hpp"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <math.h>

class BackgroundPass : public RenderPass {
private:
    float red;
    float green;
    float blue;
public:
    BackgroundPass(float r, float g, float b);
    virtual void init();
    virtual void execute();
    virtual void clean();
    virtual void time_color();
};