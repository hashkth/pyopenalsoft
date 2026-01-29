#pragma once
#include "device.h"


class Context
{
public:
    explicit Context(Device& device);
    ~Context();

    Context(const Context&) = delete;
    Context& operator=(const Context&) = delete;

    Context(Context&& other) noexcept;
    Context& operator=(Context&& other) noexcept;

    bool is_valid() const { return context_ != nullptr; }

private:
    void* context_ = nullptr;
};