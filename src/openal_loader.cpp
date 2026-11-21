#include "openal_loader.h"
#include <windows.h>

static ALCFunctions alc_;
static ALFunctions al_;

std::string get_default_library_path() 
{
    // Returns path to folder containing the .pyd file + subpath
    char path[MAX_PATH];
    path[0] = '\0';
    HMODULE hm = nullptr;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCSTR)&get_default_library_path, &hm)) {
        GetModuleFileNameA(hm, path, sizeof(path));
    }
    std::filesystem::path module_path = std::filesystem::path(path).parent_path();
    std::filesystem::path dll_subpath = (sizeof(void*) == 8) ? "Win64/soft_oal.dll" : "Win32/soft_oal.dll";
    return (module_path / dll_subpath).string();
}

bool OpenALLoader::load_library(const std::string& path) 
{
    std::string abs_path = std::filesystem::absolute(path).string();    
    lib_handle_ = static_cast<void*>(LoadLibraryA(abs_path.c_str()));
    
    if (!lib_handle_)
    {
        DWORD err = GetLastError();
        std::ostringstream oss;
        oss << "Failed to load OpenAL library at: " << abs_path << "\n"
            << "Error Code: " << err << "\n";
        if (err == ERROR_BAD_EXE_FORMAT) {
            oss << "Error: Architecture mismatch - Ensure both Python and the DLL are " 
                << (sizeof(void*) == 8 ? "64-bit" : "32-bit");
        } else {
            oss << "Error: DLL not found at specified path";
        }
        throw std::runtime_error(oss.str());
    }

    HMODULE h_mod = static_cast<HMODULE>(lib_handle_);
    bool success = true;

    #define LOAD_PROC(lib, name, target) \
        target.name = reinterpret_cast<decltype(target.name)>(GetProcAddress(lib, #name)); \
        if (!target.name) success = false;

    // Load ALC functions
    LOAD_PROC(h_mod, alcOpenDevice, alc_);
    LOAD_PROC(h_mod, alcCloseDevice, alc_);
    LOAD_PROC(h_mod, alcCreateContext, alc_);
    LOAD_PROC(h_mod, alcDestroyContext, alc_);
    LOAD_PROC(h_mod, alcMakeContextCurrent, alc_);
    LOAD_PROC(h_mod, alcGetProcAddress, alc_);
    LOAD_PROC(h_mod, alcGetIntegerv, alc_);
    LOAD_PROC(h_mod, alcGetString, alc_);

    // Load AL Buffer functions
    LOAD_PROC(h_mod, alGenBuffers, al_);
    LOAD_PROC(h_mod, alGetBufferiv, al_);
    LOAD_PROC(h_mod, alDeleteBuffers, al_);
    LOAD_PROC(h_mod, alBufferData, al_);

    // Load AL Source functions
    LOAD_PROC(h_mod, alGenSources, al_);
    LOAD_PROC(h_mod, alDeleteSources, al_);
    LOAD_PROC(h_mod, alSourcei, al_);
    LOAD_PROC(h_mod, alGetSourcei, al_);
    LOAD_PROC(h_mod, alGetSourcef, al_);
    LOAD_PROC(h_mod, alSourcef, al_);
    LOAD_PROC(h_mod, alSource3f, al_);
    LOAD_PROC(h_mod, alGetSource3f, al_);
    LOAD_PROC(h_mod, alSourcePlay, al_);
    LOAD_PROC(h_mod, alSourceStop, al_);
    LOAD_PROC(h_mod, alSourcePause, al_);
    LOAD_PROC(h_mod, alSourceRewind, al_);
    LOAD_PROC(h_mod, alSourceQueueBuffers, al_);
    LOAD_PROC(h_mod, alSourceUnqueueBuffers, al_);

    // Load AL Listener functions
    LOAD_PROC(h_mod, alListenerf, al_);
    LOAD_PROC(h_mod, alListener3f, al_);
    LOAD_PROC(h_mod, alGetListener3f, al_);
    LOAD_PROC(h_mod, alListenerfv, al_);
    LOAD_PROC(h_mod, alGetListenerf, al_);
    LOAD_PROC(h_mod, alGetListenerfv, al_);

    // Load AL Other functions
    LOAD_PROC(h_mod, alDistanceModel, al_);
    LOAD_PROC(h_mod, alGetInteger, al_);

    #undef LOAD_PROC

    if (!success)
    {
        FreeLibrary(h_mod);
        lib_handle_ = nullptr;
        return false;
    }
    return true;
}

ALCFunctions& OpenALLoader::alc()
{
    if (!initialized_)
        throw std::runtime_error("OpenAL not initialized");
    return alc_;
}

ALFunctions& OpenALLoader::al()
{
    if (!initialized_)
        throw std::runtime_error("OpenAL not initialized");
    return al_;
}

bool OpenALLoader::init(const std::string& path)
{
    std::lock_guard lock(mutex_);
    if (initialized_) return true;
    // Use soft_oal.dll shipped with pyopenalsoft if no path specified
    std::filesystem::path p = path.empty() ? get_default_library_path() : path;
    if (!load_library(p.string()))
        return false;
    initialized_ = true;
    dll_path_ = p.string();
    return true;
}

void OpenALLoader::shutdown()
{
    std::lock_guard lock(mutex_);
    if (!initialized_) return;
    if (lib_handle_)
    {
        FreeLibrary(static_cast<HMODULE>(lib_handle_));
        lib_handle_ = nullptr;
    }
    initialized_ = false;
    dll_path_.clear();
}