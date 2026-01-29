#define DR_MP3_IMPLEMENTATION
#define DR_WAV_IMPLEMENTATION
#include "audio_data.h"
#include "stb_vorbis.c"
#include "dr_mp3.h"
#include "dr_wav.h"


enum class Format { MP3, OGG, WAV, Unknown };

static Format get_format(const std::string& path)
{
    size_t dot = path.find_last_of('.');
    if (dot == std::string::npos) return Format::Unknown;
    std::string ext = path.substr(dot + 1);
    std::transform(ext.begin(), ext.end(), ext.begin(), ::tolower);
    if (ext == "mp3") return Format::MP3;
    if (ext == "ogg") return Format::OGG;
    if (ext == "wav") return Format::WAV;
    return Format::Unknown;
}

static std::vector<uint8_t> downmix_16bit(const int16_t* stereo, size_t frames)
{
    std::vector<uint8_t> mono(frames * sizeof(int16_t));
    int16_t* out = reinterpret_cast<int16_t*>(mono.data());
    for (size_t i = 0; i < frames; ++i)
    {
        int32_t avg = (static_cast<int32_t>(stereo[i * 2]) + static_cast<int32_t>(stereo[i * 2 + 1])) / 2;
        out[i] = static_cast<int16_t>(avg);
    }
    return mono;
}

AudioData::AudioData(const std::string& path, bool forceMono) 
    : sourcePath(path), forceMono(forceMono) 
{
    Format fmt = get_format(path);
    if (fmt == Format::WAV)
    {
        drwav wav;
        if (!drwav_init_file(&wav, path.c_str(), nullptr)) 
            throw std::runtime_error("WAV open error: " + path);
        this->sampleRate = wav.sampleRate;
        this->channels = (uint16_t)wav.channels;
        this->totalFrames = wav.totalPCMFrameCount;
        drwav_uninit(&wav);
    } 
    else if (fmt == Format::MP3)
    {
        drmp3 mp3;
        if (!drmp3_init_file(&mp3, path.c_str(), nullptr)) 
            throw std::runtime_error("MP3 open error: " + path);
        this->sampleRate = mp3.sampleRate;
        this->channels = (uint16_t)mp3.channels;
        this->totalFrames = drmp3_get_pcm_frame_count(&mp3);
        drmp3_uninit(&mp3);
    } 
    else if (fmt == Format::OGG)
    {
        int err;
        stb_vorbis* v = stb_vorbis_open_filename(path.c_str(), &err, nullptr);
        if (!v) throw std::runtime_error("OGG open error: " + path);
        stb_vorbis_info i = stb_vorbis_get_info(v);
        this->sampleRate = i.sample_rate;
        this->channels = (uint16_t)i.channels;
        this->totalFrames = stb_vorbis_stream_length_in_samples(v);
        stb_vorbis_close(v);
    }
    else
        throw std::runtime_error("Unsupported file extension: " + path);
    if (forceMono)
        this->channels = 1;
    this->totalSamples = this->totalFrames * this->channels;
    this->durationSeconds = static_cast<float>(this->totalFrames) / this->sampleRate;
}

std::vector<uint8_t> AudioData::decode() const
{
    switch (get_format(sourcePath))
    {
        case Format::MP3: return decodeMP3();
        case Format::OGG: return decodeOGG();
        case Format::WAV: return decodeWAV();
        default: throw std::runtime_error("Unsupported format: " + sourcePath);
    }
}

std::vector<uint8_t> AudioData::decodeWAV() const
{
    drwav wav;
    if (!drwav_init_file(&wav, sourcePath.c_str(), nullptr)) 
        throw std::runtime_error("WAV init failed: " + sourcePath);
    std::vector<int16_t> temp(totalFrames * wav.channels);
    uint64_t framesRead = drwav_read_pcm_frames_s16(&wav, totalFrames, temp.data());
    std::vector<uint8_t> result;
    if (wav.channels == 2 && forceMono)
        result = downmix_16bit(temp.data(), framesRead);
    else
    {
        result.resize(temp.size() * sizeof(int16_t));
        std::memcpy(result.data(), temp.data(), result.size());
    }
    drwav_uninit(&wav);
    return result;
}

std::vector<uint8_t> AudioData::decodeOGG() const
{
    int channels, sampleRate;
    short* output = nullptr;
    int frames = stb_vorbis_decode_filename(sourcePath.c_str(), &channels, &sampleRate, &output);
    if (frames <= 0) throw std::runtime_error("OGG decode failed: " + sourcePath);
    std::vector<uint8_t> result;
    if (channels == 2 && forceMono)
        result = downmix_16bit(output, frames);
    else
    {
        size_t size = static_cast<size_t>(frames) * channels * sizeof(int16_t);
        result.assign(reinterpret_cast<uint8_t*>(output), reinterpret_cast<uint8_t*>(output) + size);
    }
    free(output);
    return result;
}

std::vector<uint8_t> AudioData::decodeMP3() const
{
    drmp3 mp3;
    if (!drmp3_init_file(&mp3, sourcePath.c_str(), nullptr)) 
        throw std::runtime_error("MP3 init failed: " + sourcePath);
    std::vector<int16_t> temp(totalFrames * mp3.channels);
    uint64_t framesRead = drmp3_read_pcm_frames_s16(&mp3, totalFrames, temp.data());
    std::vector<uint8_t> result;
    if (mp3.channels == 2 && forceMono)
        result = downmix_16bit(temp.data(), framesRead);
    else
    {
        result.resize(temp.size() * sizeof(int16_t));
        std::memcpy(result.data(), temp.data(), result.size());
    }
    drmp3_uninit(&mp3);
    return result;
}