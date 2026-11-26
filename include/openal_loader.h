#pragma once
#include <string>
#include <mutex>
#include <stdexcept>
#include <sstream>
#include <filesystem>

struct ALCFunctions
{
    // OpenAL Context function pointer table
    void* (*alcOpenDevice)(const char*);
    int   (*alcCloseDevice)(void*);
    void* (*alcCreateContext)(void*, const int*);
    void  (*alcDestroyContext)(void*);
    int   (*alcMakeContextCurrent)(void*);
    void* (*alcGetProcAddress)(void*, const char*);
    void  (*alcGetIntegerv)(void*, int, int, int*);
    const char* (*alcGetString)(void*, int);
};

struct ALFunctions
{
    // OpenAL function pointer table
    void (*alGenBuffers)(int, unsigned int*);
    void (*alGetBufferiv)(unsigned int, int, int*);
    void (*alDeleteBuffers)(int, const unsigned int*);
    void (*alBufferData)(unsigned int, int, const void*, int, int);
    void (*alGenSources)(int, unsigned int*);
    void (*alDeleteSources)(int, const unsigned int*);
    void (*alSourcei)(unsigned int, int, int);
    void (*alGetSourcei)(unsigned int, int, int*);
    void (*alGetSourcef)(unsigned int, int, float*);
    void (*alGetSource3f)(unsigned int, int, float*, float*, float*);
    void (*alSourcef)(unsigned int, int, float);
    void (*alSource3f)(unsigned int, int, float, float, float);
    void (*alSourcePlay)(unsigned int);
    void (*alSourceStop)(unsigned int);
    void (*alSourcePause)(unsigned int);
    void (*alSourceRewind)(unsigned int);
    void (*alSourceQueueBuffers)(unsigned int, int, const unsigned int*);
    void (*alSourceUnqueueBuffers)(unsigned int, int, unsigned int*);
    void (*alListenerf)(int, float);
    void (*alListenerfv)(int, const float*);
    void (*alGetListenerf)(int, float*);
    void (*alGetListenerfv)(int, float*);
    void (*alListener3f)(int, float, float, float);
    void (*alGetListener3f)(int, float*, float*, float*);
    void (*alDistanceModel)(int);
    int  (*alGetInteger)(int);
};

class OpenALLoader
{
public:
    // Non instantiable static class
    OpenALLoader() = delete;

    static bool init(const std::string& path = "");
    static void shutdown();

    static std::string get_dll_path() { return dll_path_; }
    static bool is_initialized();

    // Function table accessors
    static ALCFunctions& alc();
    static ALFunctions& al();

private:
    // Holds the ref to loaded soft_oal.dll
    static inline void* lib_handle_ = nullptr;
    
    static inline std::mutex mutex_;
    static inline bool initialized_ = false;
    static inline std::string dll_path_;

    static bool load_library(const std::string& path);  
};