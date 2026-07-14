#include "listener.h"


constexpr int AL_POSITION    = 0x1004;
constexpr int AL_VELOCITY    = 0x1006;
constexpr int AL_ORIENTATION = 0x100F;
constexpr int AL_GAIN        = 0x100A;
constexpr int AL_DISTANCE_MODEL = 0xD000;

void Listener::set_gain(float gain)
{
    OpenALLoader::al().alListenerf(AL_GAIN, gain);
}

float Listener::get_gain()
{
    float gain;
    OpenALLoader::al().alGetListenerf(AL_GAIN, &gain);
    return gain;
}

void Listener::set_distance_model(DistanceModel model)
{
    OpenALLoader::al().alDistanceModel(static_cast<int>(model));
}

DistanceModel Listener::get_distance_model()
{
    return static_cast<DistanceModel>(OpenALLoader::al().alGetInteger(AL_DISTANCE_MODEL));
}

void Listener::set_position(float x, float y, float z)
{
    OpenALLoader::al().alListener3f(AL_POSITION, x, y, z);
}

void Listener::set_velocity(float x, float y, float z)
{
    OpenALLoader::al().alListener3f(AL_VELOCITY, x, y, z);
}

void Listener::set_orientation(float fx, float fy, float fz, float ux, float uy, float uz)
{
    float orient[6] = { fx, fy, fz, ux, uy, uz };
    OpenALLoader::al().alListenerfv(AL_ORIENTATION, orient);
}

std::tuple<float, float, float> Listener::get_position()
{
    float x, y, z;
    OpenALLoader::al().alGetListener3f(AL_POSITION, &x, &y, &z);
    return std::make_tuple(x, y, z);
}

std::tuple<float, float, float> Listener::get_velocity()
{
    float x, y, z;
    OpenALLoader::al().alGetListener3f(AL_VELOCITY, &x, &y, &z);
    return std::make_tuple(x, y, z);
}

std::tuple<float, float, float, float, float, float> Listener::get_orientation()
{
    float orient[6];
    OpenALLoader::al().alGetListenerfv(AL_ORIENTATION, orient);
    return std::make_tuple(
        orient[0], orient[1], orient[2], 
        orient[3], orient[4], orient[5]
    );
}

void Listener::reset()
{
    set_gain(1.0f);
    set_position(0.0f, 0.0f, 0.0f);
    set_velocity(0.0f, 0.0f, 0.0f);
    set_orientation(0.0f, 0.0f, -1.0f, 0.0f, 1.0f, 0.0f);
}