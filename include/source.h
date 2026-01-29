#pragma once
#include "buffer.h"


class Source
{
public:
    Source();
    ~Source();

    Source(Source&&) noexcept;
    Source& operator=(Source&&) noexcept;

    Source(const Source&) = delete;
    Source& operator=(const Source&) = delete;

    void play();
    void pause();
    void stop();
    
    bool is_playing() const;
    bool is_paused() const;
    bool is_stopped() const;

    void set_buffer(const Buffer& buffer);

    void set_looping(bool loop);
    void set_gain(float gain);
    void set_pitch(float pitch);
    void set_offset(float seconds);
    void set_relative(bool relative);
    void set_reference_distance(float distance);
    void set_rolloff_factor(float factor);
    void set_max_distance(float distance);
    
    bool  get_looping() const;
    float get_gain() const;
    float get_pitch() const;
    float get_offset() const;
    bool  get_relative() const;
    float get_reference_distance() const;
    float get_rolloff_factor() const;
    float get_max_distance() const;

    void set_position(float x, float y, float z);
    void set_velocity(float x, float y, float z);

    unsigned int id() const { return id_; }

private:
    unsigned int id_ = 0;
};