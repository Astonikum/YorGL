#include "renderer.hpp"

namespace yorgl {

Renderer::Renderer(std::unique_ptr<Backend> backend) : backend_(std::move(backend)) {
    valid_ = backend_ && backend_->init();
}

Renderer::~Renderer() {
    if (backend_) backend_->shutdown();
}

} // namespace yorgl
