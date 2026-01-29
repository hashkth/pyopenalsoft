#include "buffer.h"


constexpr int AL_FORMAT_MONO16   = 0x1101;
constexpr int AL_FORMAT_STEREO16 = 0x1103;

static int to_al_format(uint16_t channels)
{
    if (channels == 1) return AL_FORMAT_MONO16;
    if (channels == 2) return AL_FORMAT_STEREO16;
    throw std::runtime_error("Unsupported channel count: " + std::to_string(channels));
}

Buffer::Buffer(const AudioData& audio)
{
    OpenALLoader::al().alGenBuffers(1, &id_);
    if (!id_)
        throw std::runtime_error("Failed to create OpenAL buffer");
    try
    {
        auto pcm = audio.decode();
        if (pcm.empty())
            throw std::runtime_error("Decoded audio is empty for: " + audio.sourcePath);
        int format = to_al_format(audio.forceMono ? 1 : audio.channels);
        OpenALLoader::al().alBufferData(
            id_,
            format,
            pcm.data(),
            static_cast<int>(pcm.size()),
            audio.sampleRate
        );
    } 
    catch (...)
    {
        OpenALLoader::al().alDeleteBuffers(1, &id_);
        throw;
    }
}

Buffer::~Buffer()
{
    if (id_) OpenALLoader::al().alDeleteBuffers(1, &id_);
}

Buffer::Buffer(Buffer&& other) noexcept : id_(other.id_)
{
    other.id_ = 0;
}

Buffer& Buffer::operator=(Buffer&& other) noexcept
{
    if (this != &other)
    {
        if (id_) OpenALLoader::al().alDeleteBuffers(1, &id_);
        id_ = other.id_;
        other.id_ = 0;
    }
    return *this;
}