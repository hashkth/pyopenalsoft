#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <fstream>


// The implementation is self-explanatory
// Refer to dr_wav, dr_mp3 and stb_vorbis docs to understand decoding
class AudioData
{
public:
    uint32_t sampleRate = 0;
    uint16_t channels = 0;
    uint16_t bitsPerSample = 16;
    uint16_t bytesPerSample = 2;
    uint64_t totalFrames = 0;
    uint64_t totalSamples = 0;
    double durationSeconds = 0.0;
    std::string sourcePath;
    bool forceMono = false;

    AudioData() = default;

    AudioData(uint32_t sampleRate_,
              uint16_t channels_,
              uint64_t totalFrames_,
              std::string sourcePath_,
              bool forceMono_ = false)
        : sampleRate(sampleRate_),
          channels(forceMono_ ? 1 : channels_),
          totalFrames(totalFrames_),
          sourcePath(std::move(sourcePath_)),
          forceMono(forceMono_)
        {
            totalSamples = totalFrames * channels;
            durationSeconds =
                static_cast<double>(totalFrames) /
                static_cast<double>(sampleRate);
        }

    AudioData(const std::string& path, bool forceMono = false);
    
    std::vector<uint8_t> decode() const;
    std::vector<uint8_t> decodeMP3() const;
    std::vector<uint8_t> decodeOGG() const;
    std::vector<uint8_t> decodeWAV() const;
};