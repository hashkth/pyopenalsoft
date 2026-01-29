#pragma once
#include "openal_loader.h"


class Device
{
public:
    explicit Device(const std::string& name = "");
    ~Device();

    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;

    Device(Device&& other) noexcept;
    Device& operator=(Device&& other) noexcept;

    void* handle() const { return device_; }
    bool is_valid() const { return device_ != nullptr; }

private:
    void* device_ = nullptr;
};
