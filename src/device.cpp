#include "device.h"


Device::Device(const std::string& name)
{
    device_ = OpenALLoader::alc().alcOpenDevice(name.empty() ? nullptr : name.c_str());
    if (!device_)
        throw std::runtime_error("Failed to open OpenAL device");
}

Device::~Device() 
{
    if (device_)
        OpenALLoader::alc().alcCloseDevice(device_);
}

Device::Device(Device&& other) noexcept 
{
    device_ = other.device_;
    other.device_ = nullptr;
}

Device& Device::operator=(Device&& other) noexcept 
{
    if (this != &other) 
    {
        if (device_)
            OpenALLoader::alc().alcCloseDevice(device_);
        device_ = other.device_;
        other.device_ = nullptr;
    }
    return *this;
}