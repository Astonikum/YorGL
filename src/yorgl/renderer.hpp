#pragma once

#include "backend.hpp"
#include <memory>

namespace yorgl {

class Renderer {
public:
    explicit Renderer(std::unique_ptr<Backend> backend);
    ~Renderer();

    bool valid() const { return valid_; }
    Backend& backend() { return *backend_; }
    const Backend& backend() const { return *backend_; }

private:
    std::unique_ptr<Backend> backend_;
    bool valid_ = false;
};

std::unique_ptr<Backend> createBackend(BackendKind kind);

} // namespace yorgl
