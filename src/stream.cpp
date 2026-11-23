#define STB_VORBIS_HEADER_ONLY
#include "stream.h"
#include "dr_wav.h"
#include "dr_mp3.h"
#include "stb_vorbis.c"
#include <algorithm>
#include <cmath>

constexpr int AL_POSITION           = 0x1004;
constexpr int AL_VELOCITY           = 0x1006;
constexpr int AL_LOOPING            = 0x1007;
constexpr int AL_BUFFER             = 0x1009;
constexpr int AL_GAIN               = 0x100A;
constexpr int AL_PITCH              = 0x1003;
constexpr int AL_SOURCE_STATE       = 0x1010;
constexpr int AL_PLAYING            = 0x1012;
constexpr int AL_PAUSED             = 0x1013;
constexpr int AL_STOPPED            = 0x1014;
constexpr int AL_BUFFERS_PROCESSED  = 0x1015;
constexpr int AL_BUFFERS_QUEUED     = 0x1016;
constexpr int AL_SEC_OFFSET         = 0x1024;
constexpr int AL_SIZE               = 0x2004;
constexpr int AL_CHANNELS           = 0x2003;

constexpr int AL_FORMAT_MONO16      = 0x1101;
constexpr int AL_FORMAT_STEREO16    = 0x1103;

enum StreamFormat
{
    WAV,
    MP3,
    OGG
};

Stream::Stream(const std::string& path, size_t bufferSize) :
    path_(path), bufferSize_(bufferSize),
    playing_(false), looping_(false), 
    mono_(false), framesProcessed_(0)
{
    std::lock_guard<std::mutex> lock(mutex_);

    // Generate ring buffer elements and a source for playback
    OpenALLoader::al().alGenSources(1, &sourceId_);
    OpenALLoader::al().alGenBuffers(4, bufferIds_);
    
    std::string ext = path.substr(path.find_last_of(".") + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    
    // Load basic metadata
    if (ext == "wav") {
        drwav* wav = new drwav();
        if (drwav_init_file(wav, path.c_str(), nullptr)) {
            handle_ = wav; format_ = StreamFormat::WAV;
            channels_ = wav->channels; sampleRate_ = wav->sampleRate;
        }
    }
    else if (ext == "mp3") {
        drmp3* mp3 = new drmp3();
        if (drmp3_init_file(mp3, path.c_str(), nullptr)) {
            handle_ = mp3; format_ = StreamFormat::MP3;
            channels_ = mp3->channels; sampleRate_ = mp3->sampleRate;
        }
    }
    else if (ext == "ogg") {
        int err;
        stb_vorbis* ogg = stb_vorbis_open_filename(path.c_str(), &err, nullptr);
        if (ogg) {
            handle_ = ogg; format_ = StreamFormat::OGG;
            stb_vorbis_info info = stb_vorbis_get_info(ogg);
            channels_ = info.channels; sampleRate_ = info.sample_rate;
        }
    }
    
    if (!handle_) throw std::runtime_error("Stream init failed: " + path);
    else {
        uint64_t totalFrames = 0;
        if (format_ == StreamFormat::WAV) totalFrames = ((drwav*)handle_)->totalPCMFrameCount;
        else if (format_ == StreamFormat::MP3) totalFrames = drmp3_get_pcm_frame_count((drmp3*)handle_);
        else if (format_ == StreamFormat::OGG) totalFrames = stb_vorbis_stream_length_in_samples((stb_vorbis*)handle_);
        duration_ = static_cast<float>(totalFrames) / sampleRate_;
    }
    
    alFormat_ = (channels_ == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
    OpenALLoader::al().alSourcei(sourceId_, AL_LOOPING, 0); 
    OpenALLoader::al().alSourceRewind(sourceId_);
    OpenALLoader::al().alSourcei(sourceId_, AL_BUFFER, 0);
    
    for (int i = 0; i < 4; ++i) {
        if (fill_buffer(bufferIds_[i])) 
            OpenALLoader::al().alSourceQueueBuffers(sourceId_, 1, &bufferIds_[i]);
    }
    framesProcessed_ = 0;
    OpenALLoader::al().alSourceStop(sourceId_);
}

Stream::~Stream()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (sourceId_) {
        OpenALLoader::al().alSourceStop(sourceId_);
        clear_queue();
        OpenALLoader::al().alDeleteSources(1, &sourceId_);
    }
    OpenALLoader::al().alDeleteBuffers(4, bufferIds_);
    clear_handle();
}

bool Stream::fill_buffer(unsigned int alBufferId) {
    size_t samplesNeeded = bufferSize_ / sizeof(int16_t);
    std::vector<int16_t> pcm(samplesNeeded);
    size_t totalFramesRead = 0;
    
    size_t framesToRead = samplesNeeded / channels_; 
    
    while (totalFramesRead < framesToRead) {
        uint64_t framesReadThisIteration = 0;
        size_t remainingFrames = framesToRead - totalFramesRead;
        int16_t* writePtr = pcm.data() + (totalFramesRead * channels_);
        
        if (format_ == StreamFormat::WAV)
            framesReadThisIteration = drwav_read_pcm_frames_s16((drwav*)handle_, remainingFrames, writePtr);
        else if (format_ == StreamFormat::MP3)
            framesReadThisIteration = drmp3_read_pcm_frames_s16((drmp3*)handle_, remainingFrames, writePtr);
        else if (format_ == StreamFormat::OGG) {
            int samples = stb_vorbis_get_samples_short_interleaved((stb_vorbis*)handle_, channels_, writePtr, (int)(remainingFrames * channels_));
            framesReadThisIteration = (samples > 0) ? samples : 0;
        }
        
        if (framesReadThisIteration == 0) {
            if (looping_) {
                if (format_ == StreamFormat::WAV) drwav_seek_to_pcm_frame((drwav*)handle_, 0);
                else if (format_ == StreamFormat::MP3) drmp3_seek_to_pcm_frame((drmp3*)handle_, 0);
                else if (format_ == StreamFormat::OGG) stb_vorbis_seek_start((stb_vorbis*)handle_);
                if (totalFramesRead == 0) break; 
                continue;
            }
            else
                break;
        }
        totalFramesRead += framesReadThisIteration;
    }
    
    if (totalFramesRead == 0) return false;
    
    int currentFormat = alFormat_;
    size_t finalByteSize = totalFramesRead * channels_ * sizeof(int16_t);
    
    if (mono_ && channels_ == 2) {
        std::vector<int16_t> monoMix(totalFramesRead);
        for (size_t i = 0; i < totalFramesRead; ++i)  {
            int32_t left = pcm[i * 2];
            int32_t right = pcm[i * 2 + 1];
            monoMix[i] = static_cast<int16_t>((left + right) / 2);
        }
        currentFormat = AL_FORMAT_MONO16;
        finalByteSize = totalFramesRead * sizeof(int16_t);
        OpenALLoader::al().alBufferData(alBufferId, currentFormat, monoMix.data(), (int)finalByteSize, sampleRate_);
    }
    else {
        OpenALLoader::al().alBufferData(alBufferId, currentFormat, pcm.data(), (int)finalByteSize, sampleRate_);
    }
    
    return true;
}

void Stream::update()
{
    std::unique_lock<std::mutex> lock(mutex_);
    
    int processed;
    OpenALLoader::al().alGetSourcei(sourceId_, AL_BUFFERS_PROCESSED, &processed);
    
    while (processed-- > 0) {
        unsigned int bufferId = 0;
        OpenALLoader::al().alSourceUnqueueBuffers(sourceId_, 1, &bufferId);
        if (bufferId != 0) {
            int sizeInBytes = 0;
            int bufferChannels = 0;
            
            OpenALLoader::al().alGetBufferiv(bufferId, AL_SIZE, &sizeInBytes);
            OpenALLoader::al().alGetBufferiv(bufferId, AL_CHANNELS, &bufferChannels);
            
            if (bufferChannels > 0) {
                size_t framesPlayed = (sizeInBytes / sizeof(int16_t)) / bufferChannels;
                framesProcessed_ += framesPlayed;                
                uint64_t totalFileFrames = static_cast<uint64_t>(duration_ * sampleRate_);
                if (looping_ && totalFileFrames > 0)
                    framesProcessed_ %= totalFileFrames;
            }

            if (fill_buffer(bufferId))
                OpenALLoader::al().alSourceQueueBuffers(sourceId_, 1, &bufferId);
        }
    }
    
    int state;
    OpenALLoader::al().alGetSourcei(sourceId_, AL_SOURCE_STATE, &state);
    
    if (playing_ && state != AL_PLAYING) {
        int queued;
        OpenALLoader::al().alGetSourcei(sourceId_, AL_BUFFERS_QUEUED, &queued);
        if (queued > 0) {
            OpenALLoader::al().alSourcePlay(sourceId_);
        }
        else if (looping_) {
            lock.unlock(); 
            set_offset(0.0f); 
            lock.lock();
            OpenALLoader::al().alSourcePlay(sourceId_);
        }
        else {
            lock.unlock();
            stop();
        }
    }
}

void Stream::clear_handle() 
{
    if (!handle_) return;
    switch (format_) {
        case StreamFormat::WAV: delete static_cast<drwav*>(handle_); break;
        case StreamFormat::MP3: delete static_cast<drmp3*>(handle_); break;
        case StreamFormat::OGG: stb_vorbis_close(static_cast<stb_vorbis*>(handle_)); break;
    }
    handle_ = nullptr;
}

void Stream::clear_queue()
{
    OpenALLoader::al().alSourceRewind(sourceId_); 
    OpenALLoader::al().alSourcei(sourceId_, AL_BUFFER, 0); 
}

void Stream::play()
{
    std::lock_guard<std::mutex> lock(mutex_);
    playing_ = true;
    int state;
    OpenALLoader::al().alGetSourcei(sourceId_, AL_SOURCE_STATE, &state);
    if (state != AL_PLAYING) OpenALLoader::al().alSourcePlay(sourceId_);
}

void Stream::pause()
{
    std::lock_guard<std::mutex> lock(mutex_);
    playing_ = false;
    OpenALLoader::al().alSourcePause(sourceId_);
}

void Stream::stop()
{
    std::lock_guard<std::mutex> lock(mutex_);
    playing_ = false;
    OpenALLoader::al().alSourceStop(sourceId_);
    clear_queue();
    
    if (format_ == StreamFormat::WAV) drwav_seek_to_pcm_frame((drwav*)handle_, 0);
    else if (format_ == StreamFormat::MP3) drmp3_seek_to_pcm_frame((drmp3*)handle_, 0);
    else if (format_ == StreamFormat::OGG) stb_vorbis_seek_start((stb_vorbis*)handle_);
    
    for (int i = 0; i < 4; ++i) {
        if (fill_buffer(bufferIds_[i]))
            OpenALLoader::al().alSourceQueueBuffers(sourceId_, 1, &bufferIds_[i]);
    }
    framesProcessed_ = 0;
}

bool Stream::is_playing() const
{
    int state;
    OpenALLoader::al().alGetSourcei(sourceId_, AL_SOURCE_STATE, &state);
    return state == AL_PLAYING;
}

bool Stream::is_paused() const
{
    int state;
    OpenALLoader::al().alGetSourcei(sourceId_, AL_SOURCE_STATE, &state);
    return state == AL_PAUSED;
}

bool Stream::is_stopped() const
{
    int state;
    OpenALLoader::al().alGetSourcei(sourceId_, AL_SOURCE_STATE, &state);
    return state == AL_STOPPED;
}

void Stream::set_gain(float gain)
{
    OpenALLoader::al().alSourcef(sourceId_, AL_GAIN, (gain < 0.0f) ? 0.0f : gain);
}

float Stream::get_gain() const
{
    float gain;
    OpenALLoader::al().alGetSourcef(sourceId_, AL_GAIN, &gain);
    return gain;
}

void Stream::set_pitch(float pitch)
{
    OpenALLoader::al().alSourcef(sourceId_, AL_PITCH, pitch);
}

float Stream::get_pitch() const
{
    float p;
    OpenALLoader::al().alGetSourcef(sourceId_, AL_PITCH, &p);
    return p;
}

void Stream::set_offset(float seconds) {
    std::lock_guard<std::mutex> lock(mutex_);
    uint64_t frame = static_cast<uint64_t>(seconds * sampleRate_);
    if (format_ == StreamFormat::WAV) drwav_seek_to_pcm_frame((drwav*)handle_, frame);
    else if (format_ == StreamFormat::MP3) drmp3_seek_to_pcm_frame((drmp3*)handle_, frame);
    else if (format_ == StreamFormat::OGG) stb_vorbis_seek_frame((stb_vorbis*)handle_, (int)frame);
    clear_queue();
    for (int i = 0; i < 4; ++i) {
        if (fill_buffer(bufferIds_[i]))
            OpenALLoader::al().alSourceQueueBuffers(sourceId_, 1, &bufferIds_[i]);
    }
    framesProcessed_ = frame;
    if (playing_)
        OpenALLoader::al().alSourcePlay(sourceId_);
}

float Stream::get_offset() const
{
    std::lock_guard<std::mutex> lock(mutex_);
    float bufferOffset;
    OpenALLoader::al().alGetSourcef(sourceId_, AL_SEC_OFFSET, &bufferOffset);     
    float totalOffset = (static_cast<float>(framesProcessed_) / sampleRate_) + bufferOffset;
    return std::clamp(totalOffset, 0.0f, duration_);
}

void Stream::set_mono(bool enable)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (mono_ == enable) return;
    mono_ = enable;
    float currentPos = get_offset();
    set_offset(currentPos); 
}

float Stream::get_progress() const
{
    if (duration_ <= 0.0f) return 0.0f;
    float progress = get_offset() / duration_;
    return (progress > 1.0f) ? 1.0f : progress;
}

void Stream::set_position(float x, float y, float z)
{
    OpenALLoader::al().alSource3f(sourceId_, AL_POSITION, x, y, z);
}

void Stream::set_velocity(float x, float y, float z)
{
    OpenALLoader::al().alSource3f(sourceId_, AL_VELOCITY, x, y, z);
}

std::tuple<float, float, float> Stream::get_position() const
{
    float x, y, z;
    OpenALLoader::al().alGetSource3f(sourceId_, AL_POSITION, &x, &y, &z);
    return std::make_tuple(x, y, z);
}

std::tuple<float, float, float> Stream::get_velocity() const
{
    float x, y, z;
    OpenALLoader::al().alGetSource3f(sourceId_, AL_VELOCITY, &x, &y, &z);
    return std::make_tuple(x, y, z);
}