#include "Application.hpp"
#include <iostream>
using std::cerr;
using std::endl;

int Application::initialize(int width, int height) {
    if (!glfwInit()) {
        cerr << "Failed to glfwInit" << endl;
        return -1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(width, height, "App", NULL, NULL);
    if (!window) {
        cerr << "Failed to CreateWindow" << endl;
        return -1;
    }
    glfwMakeContextCurrent(window);
    if (!gladLoadGL()) {
        cerr << "Failed to LoadGL" << endl;
        glfwDestroyWindow(window);
        glfwTerminate();
        return -1;
    }
    return 0;
}

Application::Application(int width, int height) {
    if (initialize(width, height) != 0) {
        cerr << "app init error" << endl;
        clean_up();
    }
}

void Application::clean_up() {
    if (window) {
        glfwDestroyWindow(window);
        window = nullptr;
    }
    glfwTerminate();
}

bool Application::add_render_pass(std::unique_ptr<RenderPass> pass) {
    v_render_pass.emplace_back(std::move(pass));
    return true;
}

int Application::run() {
    glfwSwapInterval(1);
    while (!glfwWindowShouldClose(window)) {
        for (auto& event:v_event) {
            event->execute(window);
        }
        for (auto& pass:v_render_pass) {
            pass->execute();
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    return 0;
}

Application::~Application() {
    clean_up();
}