#pragma once
#include "audio_data.h"
#include "openal_loader.h"


class Stream
{
public:
    Stream(const std::string& path, size_t bufferSize = 65536);
    ~Stream();

    void update();
    void play();
    void pause();
    void stop();

    void set_gain(float gain);
    float get_gain() const;

    void set_pitch(float pitch);
    float get_pitch() const;

    void set_offset(float seconds);
    float get_offset() const;
    
    void set_looping(bool loop) { looping_ = loop; }
    bool get_looping() const { return looping_; }
    
    void set_surround(bool enable);
    bool get_surround() const { return surround_; }
    
    void set_position(float x, float y, float z);
    void set_velocity(float x, float y, float z);
    
    float get_progress() const;
    float get_total_duration() const { return duration_; }

private:
unsigned int sourceId_ = 0;
    unsigned int bufferIds_[4] = {0, 0, 0, 0};

    void* handle_ = nullptr;
    int format_ = 0;
    int alFormat_ = 0;
    int sampleRate_ = 0;
    int channels_ = 0;

    std::string path_;
    size_t bufferSize_;
    bool playing_ = false;
    bool looping_ = false;
    bool surround_ = false;

    uint64_t samplesProcessed_ = 0;
    double currentPlayheadBase_ = 0.0;
    float duration_ = 0.0f;

    bool fill_buffer(unsigned int alBufferId);
    void clear_handle();
    void clear_queue();
};