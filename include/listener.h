#pragma once
#include "openal_loader.h"


enum class DistanceModel
{
    None = 0,
    Inverse = 0xD001,
    InverseClamped = 0xD002,
    Linear = 0xD003,
    LinearClamped = 0xD004,
    Exponent = 0xD005,
    ExponentClamped = 0xD006
};

class Listener
{
public:
    static void set_gain(float gain);
    static float get_gain();
    static void set_distance_model(DistanceModel model);
    static DistanceModel get_distance_model();
    
    static void set_position(float x, float y, float z);
    static void set_velocity(float x, float y, float z);
    static void set_orientation(float fx, float fy, float fz, float ux, float uy, float uz);
    static void reset();
};