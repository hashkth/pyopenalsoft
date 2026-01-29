#include "source.h"


constexpr int AL_BUFFER   = 0x1009;
constexpr int AL_LOOPING  = 0x1007;
constexpr int AL_GAIN     = 0x100A;
constexpr int AL_PITCH    = 0x1003;
constexpr int AL_POSITION = 0x1004;
constexpr int AL_VELOCITY = 0x1006;
constexpr int AL_SOURCE_RELATIVE = 0x202;
constexpr int AL_SEC_OFFSET      = 0x1024;

constexpr int AL_REFERENCE_DISTANCE = 0x1020;
constexpr int AL_ROLLOFF_FACTOR     = 0x1021;
constexpr int AL_MAX_DISTANCE       = 0x1023;

constexpr int AL_SOURCE_STATE = 0x1010;
constexpr int AL_PLAYING      = 0x1012;
constexpr int AL_PAUSED       = 0x1013;
constexpr int AL_STOPPED      = 0x1014;

Source::Source()
{
    OpenALLoader::al().alGenSources(1, &id_);
    if (!id_)
        throw std::runtime_error("Failed to create OpenAL source");
}

Source::~Source()
{
    if (id_)
        OpenALLoader::al().alDeleteSources(1, &id_);
}

Source::Source(Source&& other) noexcept
{
    id_ = other.id_;
    other.id_ = 0;
}

Source& Source::operator=(Source&& other) noexcept
{
    if (this != &other)
    {
        if (id_)
            OpenALLoader::al().alDeleteSources(1, &id_);
        id_ = other.id_;
        other.id_ = 0;
    }
    return *this;
}

void Source::play()
{
    if (is_playing()) return;
    OpenALLoader::al().alSourcePlay(id_);
}

void Source::pause()
{
    OpenALLoader::al().alSourcePause(id_);
}

void Source::stop()
{
    OpenALLoader::al().alSourceStop(id_);
}

bool Source::is_playing() const
{
    int state;
    OpenALLoader::al().alGetSourcei(id_, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

bool Source::is_paused() const
{
    int state;
    OpenALLoader::al().alGetSourcei(id_, AL_SOURCE_STATE, &state);
    return state == AL_PAUSED;
}

bool Source::is_stopped() const
{
    int state;
    OpenALLoader::al().alGetSourcei(id_, AL_SOURCE_STATE, &state);
    return state == AL_STOPPED;
}

void Source::set_buffer(const Buffer& buffer)
{
    OpenALLoader::al().alSourcei(id_, AL_BUFFER, static_cast<int>(buffer.id()));
}

void Source::set_looping(bool loop)
{
    OpenALLoader::al().alSourcei(id_, AL_LOOPING, loop ? 1 : 0);
}

void Source::set_gain(float gain)
{
    float g = (gain < 0.0f) ? 0.0f : gain;
    OpenALLoader::al().alSourcef(id_, AL_GAIN, g);
}

void Source::set_pitch(float pitch)
{
    float p = (pitch < 0.001f) ? 0.001f : pitch;
    OpenALLoader::al().alSourcef(id_, AL_PITCH, p);
}

void Source::set_offset(float seconds)
{
    OpenALLoader::al().alSourcef(id_, AL_SEC_OFFSET, seconds);
}

void Source::set_relative(bool relative)
{
    OpenALLoader::al().alSourcei(id_, AL_SOURCE_RELATIVE, relative ? 1 : 0);
}

void Source::set_reference_distance(float distance)
{
    OpenALLoader::al().alSourcef(id_, AL_REFERENCE_DISTANCE, distance);
}

void Source::set_rolloff_factor(float factor)
{
    OpenALLoader::al().alSourcef(id_, AL_ROLLOFF_FACTOR, factor);
}

void Source::set_max_distance(float distance)
{
    OpenALLoader::al().alSourcef(id_, AL_MAX_DISTANCE, distance);
}

bool Source::get_looping() const
{
    int loop;
    OpenALLoader::al().alGetSourcei(id_, AL_LOOPING, &loop);
    return loop == 1;
}

float Source::get_gain() const
{
    float gain;
    OpenALLoader::al().alGetSourcef(id_, AL_GAIN, &gain);
    return gain;
}

float Source::get_pitch() const
{
    float pitch;
    OpenALLoader::al().alGetSourcef(id_, AL_PITCH, &pitch);
    return pitch;
}

float Source::get_offset() const
{
    float seconds;
    OpenALLoader::al().alGetSourcef(id_, AL_SEC_OFFSET, &seconds);
    return seconds;
}

bool Source::get_relative() const
{
    int relative;
    OpenALLoader::al().alGetSourcei(id_, AL_SOURCE_RELATIVE, &relative);
    return relative == 1;
}

float Source::get_reference_distance() const
{
    float d;
    OpenALLoader::al().alGetSourcef(id_, AL_REFERENCE_DISTANCE, &d);
    return d;
}

float Source::get_rolloff_factor() const
{
    float f;
    OpenALLoader::al().alGetSourcef(id_, AL_ROLLOFF_FACTOR, &f);
    return f;
}

float Source::get_max_distance() const
{
    float d;
    OpenALLoader::al().alGetSourcef(id_, AL_MAX_DISTANCE, &d);
    return d;
}

void Source::set_position(float x, float y, float z)
{
    OpenALLoader::al().alSource3f(id_, AL_POSITION, x, y, z);
}

void Source::set_velocity(float x, float y, float z)
{
    OpenALLoader::al().alSource3f(id_, AL_VELOCITY, x, y, z);
}