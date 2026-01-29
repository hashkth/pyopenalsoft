#pragma once
#include <string>
#include <mutex>
#include <stdexcept>
#include <sstream>
#include <filesystem>
#include "windows.h"


// Prototypes for all ALC functions we wish to import from .dll
struct ALCFunctions
{
    // Device functions
    void* (*alcOpenDevice)(const char*);
    int   (*alcCloseDevice)(void*);

    // Context functions
    void* (*alcCreateContext)(void*, const int*);
    void  (*alcDestroyContext)(void*);
    int   (*alcMakeContextCurrent)(void*);
};

// Prototypes for all AL functions we wish to import from .dll
struct ALFunctions
{
    // Buffer functions
    void (*alGenBuffers)(int, unsigned int*);
    void (*alDeleteBuffers)(int, const unsigned int*);
    void (*alBufferData)(unsigned int, int, const void*, int, int);

    // Source functions
    void (*alGenSources)(int, unsigned int*);
    void (*alDeleteSources)(int, const unsigned int*);
    void (*alSourcei)(unsigned int, int, int);
    void (*alGetSourcei)(unsigned int, int, int*);
    void (*alGetSourcef)(unsigned int, int, float*);
    void (*alSourcef)(unsigned int, int, float);
    void (*alSource3f)(unsigned int, int, float, float, float);
    void (*alSourcePlay)(unsigned int);
    void (*alSourceStop)(unsigned int);
    void (*alSourcePause)(unsigned int);
    void (*alSourceRewind)(unsigned int);
    void (*alSourceQueueBuffers)(unsigned int, int, const unsigned int*);
    void (*alSourceUnqueueBuffers)(unsigned int, int, unsigned int*);
    
    // Listener functions
    void (*alListenerf)(int, float);
    void (*alListenerfv)(int, const float*);
    void (*alGetListenerf)(int, float*);
    void (*alGetListenerfv)(int, float*);
    void (*alListener3f)(int, float, float, float);

    // Other functions
    void (*alDistanceModel)(int);
    int  (*alGetInteger)(int);
};

class OpenALLoader
{
public:
    // Initialise from the provided .dll path
    static bool init(const std::string& path = "");

    // Free resources and reset state
    static void shutdown();

    std::string get_dll_path() { return dll_path_; }

    // AL and ALC function accessors
    static ALCFunctions& alc();
    static ALFunctions& al();

private:    
    // Load the functions from the .dll at given path
    static bool load_library(const std::string& path);

    static HMODULE lib_handle_;
    static std::mutex mutex_;
    static inline bool initialized_ = false;
    static inline std::string dll_path_;
};
