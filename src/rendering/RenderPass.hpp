#pragma once
#include <string>
#include <core/geometry.hpp>
#include <memory>
#include <vector>

class RenderPass {
public:
    std::string pass_name;
    std::vector<std::unique_ptr<Mesh>> v_mesh;
    virtual void init() {
        pass_name = "Default";
    }
    virtual void execute() = 0;
    virtual void clean() = 0;

    virtual std::string get_pass_name() const {
        return pass_name;
    }
};