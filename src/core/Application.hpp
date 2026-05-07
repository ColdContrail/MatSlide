#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <memory>
#include "Event.hpp"
#include "ParameterManager.hpp"
#include "rendering/RenderPass.hpp"

class Application {
protected:
    GLFWwindow* window;
    int initialize(int width, int height);
    virtual void clean_up();

    std::vector<std::unique_ptr<Event>> v_event;
    std::vector<std::unique_ptr<RenderPass>> v_render_pass;
    ParameterManager paramManager;

public:
    Application(int width = 1920, int height = 1080);
    virtual ~Application();
    virtual int run();

    bool add_event(std::unique_ptr<Event> event);
    bool add_render_pass(std::unique_ptr<RenderPass> pass);
    ParameterManager& getParamManager();

};

