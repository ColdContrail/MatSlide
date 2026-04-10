#pragma once
#include "MaterialParameters.hpp"
#include <functional>
#include <vector>

class ParameterManager {
public:
    using Callback = std::function<void(const MaterialParameters&)>;

    MaterialParameters& getParams();
    const MaterialParameters& getParams() const;

    void setDirty(bool dirty);
    bool isDirty() const;

    void subscribe(Callback cb);
    void notify();

    void renderGUI();

private:
    MaterialParameters params;
    bool dirty = false;
    std::vector<Callback> subscribers;
};
