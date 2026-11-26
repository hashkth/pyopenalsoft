#pragma once
#include "openal_loader.h"
#include <vector>

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
    std::vector<std::string> list_devices();

    bool is_connected() const;
    bool reopen(const std::string& name = "");
    void close();

private:
    // Audio device handle
    void* device_ = nullptr;
};
