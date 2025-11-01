#include "BackgroundPass.hpp"

BackgroundPass::BackgroundPass(float r, float g, float b) :red(r), green(g), blue(b) {
    this->pass_name = "background";
}
void BackgroundPass::init() {
    ;
}
void BackgroundPass::execute() {
    time_color();
    glClearColor(red, green, blue, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
}
void BackgroundPass::clean() {
    ;
}
void BackgroundPass::time_color() {
    float time = glfwGetTime();
    red = sin(time * 0.5f) * 0.1f + 0.9f;
    green = sin(time * 0.3f + 2.0f) * 0.1f + 0.9f;
    blue = sin(time * 0.7f + 4.0f) * 0.1f + 0.9f;
}