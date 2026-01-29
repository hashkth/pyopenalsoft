#include "context.h"


Context::Context(Device& device)
{
    auto& alc = OpenALLoader::alc();
    context_ = alc.alcCreateContext(device.handle(), nullptr);
    if (!context_)
        throw std::runtime_error("Failed to create OpenAL context");

    if (!alc.alcMakeContextCurrent(context_))
    {
        alc.alcDestroyContext(context_);
        context_ = nullptr;
        throw std::runtime_error("Failed to make OpenAL context current");
    }
}

Context::~Context()
{
    if (context_)
    {
        auto& alc = OpenALLoader::alc();
        alc.alcMakeContextCurrent(nullptr);
        alc.alcDestroyContext(context_);
    }
}

Context::Context(Context&& other) noexcept
{
    context_ = other.context_;
    other.context_ = nullptr;
}

Context& Context::operator=(Context&& other) noexcept
{
    if (this != &other)
    {
        if (context_)
        {
            auto& alc = OpenALLoader::alc();
            alc.alcMakeContextCurrent(nullptr);
            alc.alcDestroyContext(context_);
        }
        context_ = other.context_;
        other.context_ = nullptr;
    }
    return *this;
}