#include "context.h"


Context::Context(Device& device)
{
    auto& alc = OpenALLoader::alc();
    context_ = alc.alcCreateContext(device.handle(), nullptr);
    if (!context_)
        throw std::runtime_error("Failed to create OpenAL context");

    if (!alc.alcMakeContextCurrent(context_)) {
        alc.alcDestroyContext(context_);
        context_ = nullptr;
        throw std::runtime_error("Failed to make OpenAL context current");
    }
    OpenALLoader::register_context(this);
}

void Context::close()
{
    if (context_) {
        if (OpenALLoader::is_initialized()) {
            auto& alc = OpenALLoader::alc();
            alc.alcMakeContextCurrent(nullptr);
            alc.alcDestroyContext(context_);
        }
        context_ = nullptr;
    }
}

Context::~Context()
{
    close();
    OpenALLoader::unregister_context(this);
}

Context::Context(Context&& other) noexcept
{
    context_ = other.context_;
    other.context_ = nullptr;
    if (context_) {
        OpenALLoader::register_context(this);
    }
}

Context& Context::operator=(Context&& other) noexcept
{
    if (this != &other) {
        close();
        context_ = other.context_;
        other.context_ = nullptr;
        if (context_) {
            OpenALLoader::register_context(this);
        }
    }
    return *this;
}