#include "openal_loader.h"


static ALCFunctions alc_;
static ALFunctions al_;
std::mutex OpenALLoader::mutex_;
HMODULE OpenALLoader::lib_handle_ = nullptr;

std::string get_default_library_path() 
{
    char path[MAX_PATH];
    HMODULE hm = NULL;
    if (GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | 
                           GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                           (LPCSTR)&get_default_library_path, &hm)) 
    {
        GetModuleFileNameA(hm, path, sizeof(path));
    }
    std::filesystem::path module_path = std::filesystem::path(path).parent_path();
    std::filesystem::path dll_subpath = (sizeof(void*) == 8) ? "Win64/soft_oal.dll" : "Win32/soft_oal.dll";
    return (module_path / dll_subpath).string();
}

bool OpenALLoader::load_library(const std::string& path) 
{
    std::string abs_path = std::filesystem::absolute(path).string();
    lib_handle_ = LoadLibraryA(abs_path.c_str());
    if (!lib_handle_)
    {
        DWORD err = GetLastError();
        std::ostringstream oss;
        oss << "Failed to load OpenAL library at: " << abs_path << "\n"
            << "Error Code: " << err << "\n";
        if (err == ERROR_BAD_EXE_FORMAT)
        {
            oss << "Error: Architecture mismatch - Ensure both Python and the DLL are " 
                << (sizeof(void*) == 8 ? "64-bit" : "32-bit");
        }
        else
        {
            oss << "Error: DLL not found at specified path";
        }
        throw std::runtime_error(oss.str());
    }

    #define LOAD_PROC(lib, name, target) \
        target.name = reinterpret_cast<decltype(target.name)>(GetProcAddress(lib, #name)); \
        if (!target.name) success = false;

    bool success = true;

    // Load ALC functions
    LOAD_PROC(lib_handle_, alcOpenDevice, alc_);
    LOAD_PROC(lib_handle_, alcCloseDevice, alc_);
    LOAD_PROC(lib_handle_, alcCreateContext, alc_);
    LOAD_PROC(lib_handle_, alcDestroyContext, alc_);
    LOAD_PROC(lib_handle_, alcMakeContextCurrent, alc_);

    // Load AL Buffer functions
    LOAD_PROC(lib_handle_, alGenBuffers, al_);
    LOAD_PROC(lib_handle_, alDeleteBuffers, al_);
    LOAD_PROC(lib_handle_, alBufferData, al_);

    // Load AL Source functions
    LOAD_PROC(lib_handle_, alGenSources, al_);
    LOAD_PROC(lib_handle_, alDeleteSources, al_);
    LOAD_PROC(lib_handle_, alSourcei, al_);
    LOAD_PROC(lib_handle_, alGetSourcei, al_);
    LOAD_PROC(lib_handle_, alGetSourcef, al_);
    LOAD_PROC(lib_handle_, alSourcef, al_);
    LOAD_PROC(lib_handle_, alSource3f, al_);
    LOAD_PROC(lib_handle_, alSourcePlay, al_);
    LOAD_PROC(lib_handle_, alSourceStop, al_);
    LOAD_PROC(lib_handle_, alSourcePause, al_);
    LOAD_PROC(lib_handle_, alSourceRewind, al_);
    LOAD_PROC(lib_handle_, alSourceQueueBuffers, al_);
    LOAD_PROC(lib_handle_, alSourceUnqueueBuffers, al_);

    // Load AL Listener functions
    LOAD_PROC(lib_handle_, alListenerf, al_);
    LOAD_PROC(lib_handle_, alListener3f, al_);
    LOAD_PROC(lib_handle_, alListenerfv, al_);
    LOAD_PROC(lib_handle_, alGetListenerf, al_);
    LOAD_PROC(lib_handle_, alGetListenerfv, al_);

    // Load AL Other functions
    LOAD_PROC(lib_handle_, alDistanceModel, al_);
    LOAD_PROC(lib_handle_, alGetInteger, al_);

    #undef LOAD_PROC

    if (!success)
    {
        FreeLibrary(lib_handle_);
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
        FreeLibrary(lib_handle_);
        lib_handle_ = nullptr;
    }
    initialized_ = false;
    dll_path_.clear();
}