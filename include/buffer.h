#pragma once
#include "audio_data.h"
#include "openal_loader.h"


class Buffer
{
public:
    Buffer(const AudioData& audio);
    ~Buffer();

    Buffer(const Buffer&) = delete;
    Buffer& operator=(const Buffer&) = delete;

    Buffer(Buffer&& other) noexcept;
    Buffer& operator=(Buffer&& other) noexcept;

    unsigned int id() const { return id_; }
    
private:
    unsigned int id_ = 0;
};
