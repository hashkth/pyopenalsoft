#include "device.h"

// Definition for querying reopen func later from loaded DLL
typedef char (*LPALCREOPENDEVICESOFT)(void* device, const char* name, const int* attribs);

constexpr int ALC_ALL_DEVICES_SPECIFIER = 0x1013;
constexpr int ALC_CONNECTED = 0x313;

Device::Device(const std::string& name)
{
    device_ = OpenALLoader::alc().alcOpenDevice(name.empty() ? nullptr : name.c_str());
    if (!device_)
        throw std::runtime_error("Failed to open OpenAL device");
}

Device::~Device() 
{
    if (device_) {
        if (OpenALLoader::is_initialized()) {
            OpenALLoader::alc().alcCloseDevice(device_);
        }
        device_ = nullptr;
    }
}

Device::Device(Device&& other) noexcept 
{
    device_ = other.device_;
    other.device_ = nullptr;
}

Device& Device::operator=(Device&& other) noexcept 
{
    if (this != &other) {
        if (device_) {
            if (OpenALLoader::is_initialized()) {
                OpenALLoader::alc().alcCloseDevice(device_);
            }
        }
        device_ = other.device_;
        other.device_ = nullptr;
    }
    return *this;
}

bool Device::is_connected() const
{
    if (!device_) return false;
    int connected = 1;
    OpenALLoader::alc().alcGetIntegerv(device_, ALC_CONNECTED, 1, &connected);
    return connected != 0;
}

std::vector<std::string> Device::list_devices()
{
    std::vector<std::string> devices;
    const char* list = OpenALLoader::alc().alcGetString(nullptr, ALC_ALL_DEVICES_SPECIFIER);
    if (!list) return devices;
    while (*list != '\0')
    {
        devices.emplace_back(list);
        list += devices.back().size() + 1;
    }
    return devices;
}

bool Device::reopen(const std::string& name)
{
    if (!device_) return false;
    auto& alc = OpenALLoader::alc();
    // Query extension dynamically via alcGetProcAddress
    auto alcReopenDeviceSOFT = reinterpret_cast<LPALCREOPENDEVICESOFT>(
        alc.alcGetProcAddress(device_, "alcReopenDeviceSOFT")
    );
    if (alcReopenDeviceSOFT) {
        const char* targetDevice = name.empty() ? nullptr : name.c_str();
        return alcReopenDeviceSOFT(device_, targetDevice, nullptr) != 0;
    }
    return false;
}