#pragma once
#include <cstdint>
#include <string>
#include <vector>
#include <stdexcept>
#include <algorithm>
#include <fstream>
#include <memory>
#include <cstring>

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
    bool forceMono = false; // forcibly downmixes to a single channel if true

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
            durationSeconds = static_cast<double>(totalFrames) / sampleRate;
        }

    AudioData(const std::string& path, bool forceMono = false);
    
    std::vector<int16_t> decode() const;
    std::vector<int16_t> decodeMP3() const;
    std::vector<int16_t> decodeOGG() const;
    std::vector<int16_t> decodeWAV() const;
};