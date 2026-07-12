#include "device.h"

// Define function signature pointer types locally for OpenAL Soft extensions
typedef int (*LPALCGETINTEGERV)(void* device, int param, int size, int* values);
typedef char (*LPALCREOPENDEVICESOFT)(void* device, const char* name, const int* attribs);

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
    if (this != &other) {
        if (device_)
            OpenALLoader::alc().alcCloseDevice(device_);
        device_ = other.device_;
        other.device_ = nullptr;
    }
    return *this;
}

bool Device::is_connected() const
{
    if (!device_) return false;
    int connected = 1;
    OpenALLoader::alc().alcGetIntegerv(device_, 0x313, 1, &connected); // 0x313 is ALC_CONNECTED
    return connected != 0;
}

bool Device::reopen(const std::string& name)
{
    if (!device_) return false;
    auto& alc = OpenALLoader::alc();    
    auto alcReopenDeviceSOFT = reinterpret_cast<LPALCREOPENDEVICESOFT>(
        alc.alcGetProcAddress(device_, "alcReopenDeviceSOFT")
    );
    if (alcReopenDeviceSOFT) {
        const char* targetDevice = name.empty() ? nullptr : name.c_str();
        return alcReopenDeviceSOFT(device_, targetDevice, nullptr) != 0;
    }
    return false;
}