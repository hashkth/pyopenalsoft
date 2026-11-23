#pragma once
#include "audio_data.h"
#include "openal_loader.h"
#include <tuple>
#include <mutex>

class Stream
{
public:
    Stream(const std::string& path, size_t bufferSize = 65536);
    ~Stream();

    void update();
    void play();
    void pause();
    void stop();

    bool is_playing() const;
    bool is_paused() const;
    bool is_stopped() const;

    void set_gain(float gain);
    void set_pitch(float pitch);
    void set_offset(float seconds);
    void set_looping(bool loop) { looping_ = loop; }
    void set_mono(bool enable);
    void set_position(float x, float y, float z);
    void set_velocity(float x, float y, float z);
    
    float get_gain() const;
    float get_pitch() const;
    float get_offset() const;
    bool get_looping() const { return looping_; }
    bool get_mono() const { return mono_; }    
    float get_progress() const;
    float get_total_duration() const { return duration_; }
    std::tuple<float, float, float> get_position() const;
    std::tuple<float, float, float> get_velocity() const;

private:
    unsigned int sourceId_ = 0;

    // Ring buffer ids
    unsigned int bufferIds_[4] = {0, 0, 0, 0};

    // Pointer to decoder instance: drwav* / drmp3* / stb_vorbis*
    void* handle_ = nullptr;

    int format_ = 0;
    int alFormat_ = 0;
    int sampleRate_ = 0;
    int channels_ = 0;

    std::string path_;
    size_t bufferSize_;
    bool playing_ = false;
    bool looping_ = false;
    bool mono_ = false;

    uint64_t framesProcessed_ = 0; 
    float duration_ = 0.0f;

    mutable std::mutex mutex_;

    bool fill_buffer(unsigned int alBufferId);
    void clear_handle();
    void clear_queue();
};