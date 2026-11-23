#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <pybind11/complex.h>
#include <pybind11/chrono.h>
#include "openal_loader.h"
#include "device.h"
#include "context.h"
#include "buffer.h"
#include "source.h"
#include "listener.h"
#include "stream.h"

namespace py = pybind11;

PYBIND11_MODULE(pyopenalsoft, m) {
    m.doc() = "Python bindings for OpenAL Soft.";

    m.def("init", [](const std::optional<std::string>& path) -> bool { 
            return OpenALLoader::init(path.value_or("")); 
        }, py::arg("path") = py::none(),
        "Load OpenAL Soft from path, or search default paths if omitted.");
    m.def("shutdown", &OpenALLoader::shutdown,
        "Unload OpenAL Soft and reset loader state.");
    m.def("get_dll_path", &OpenALLoader::get_dll_path,
        "Return the path of the loaded OpenAL Soft library.");

    py::class_<Device>(m, "Device", "Audio output device handle.")
        .def(py::init<const std::string&>(), py::arg("name") = "",
            "Open an audio device by name, or the default device if empty.")
        .def_property_readonly("is_connected", &Device::is_connected,
            "Whether the device is currently connected.")
        .def("reopen", &Device::reopen, py::arg("name") = "",
            "Reopen the device with name, or the default device if empty.")
        .def("list_devices", &Device::list_devices,
            "Return available audio device names.");

    py::class_<Context>(m, "Context", "OpenAL rendering context bound to a Device.")
        .def(py::init<Device&>(), py::keep_alive<1, 2>(),
            "Create and activate a context for device.");

    py::class_<AudioData>(m, "AudioData", "Metadata and PCM decoder for WAV, MP3, and OGG files.")
        .def(py::init<const std::string&, bool>(), 
            py::arg("path"), 
            py::arg("mono") = false,
            "Load audio metadata from path, optionally downmixing to mono.")
        .def_property_readonly("sample_rate", [](const AudioData& a) { return a.sampleRate; },
            "Sample rate in Hz.")
        .def_property_readonly("channels", [](const AudioData& a) { return a.channels; },
            "Number of channels (1 if mono is enabled).")
        .def_property_readonly("bits_per_sample", [](const AudioData& a) { return a.bitsPerSample; },
            "Bits per sample.")
        .def_property_readonly("bytes_per_sample", [](const AudioData& a) { return a.bytesPerSample; },
            "Bytes per sample.")
        .def_property_readonly("total_frames", [](const AudioData& a) { return a.totalFrames; },
            "Total number of sample frames.")
        .def_property_readonly("total_samples", [](const AudioData& a) { return a.totalSamples; },
            "Total number of samples across all channels.")
        .def_property_readonly("duration", [](const AudioData& a) { return a.durationSeconds; },
            "Duration in seconds.")
        .def_property_readonly("path", [](const AudioData& a) { return a.sourcePath; },
            "Source file path.")
        .def_property_readonly("mono", [](const AudioData& a) { return a.forceMono; },
            "Whether mono downmixing is enabled.")
        .def("decode", [](const AudioData& a) {
            auto data = a.decode();
            return py::bytes(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(int16_t));
        }, "Decode the audio file into raw 16-bit signed PCM bytes.");

    py::class_<Buffer>(m, "Buffer", "OpenAL buffer holding decoded audio data.")
        .def(py::init<const AudioData&>(),
            "Decode audio_data and upload it into a new OpenAL buffer.");

    py::class_<Source>(m, "Source", "3D audio emitter that plays a Buffer.")
        .def(py::init<>(), "Create an audio source.")
        .def("play", &Source::play, "Start or resume playback.")
        .def("pause", &Source::pause, "Pause playback.")
        .def("stop", &Source::stop, "Stop playback and rewind to the start.")
        .def("reset", &Source::reset, "Reset source state and properties.")
        .def_property("buffer", &Source::get_buffer, &Source::set_buffer,
            "Attached audio buffer.")
        .def_property("gain", &Source::get_gain, &Source::set_gain,
            "Source gain (volume multiplier).")
        .def_property("pitch", &Source::get_pitch, &Source::set_pitch,
            "Pitch multiplier.")
        .def_property("offset", &Source::get_offset, &Source::set_offset,
            "Playback position in seconds.")
        .def_property("looping", &Source::get_looping, &Source::set_looping,
            "Whether playback loops.")
        .def_property("relative", &Source::get_relative, &Source::set_relative,
            "Whether position and velocity are relative to the listener.")
        .def_property("reference_distance", &Source::get_reference_distance, &Source::set_reference_distance,
            "Distance at which attenuation begins.")
        .def_property("rolloff_factor", &Source::get_rolloff_factor, &Source::set_rolloff_factor,
            "Distance attenuation strength.")
        .def_property("max_distance", &Source::get_max_distance, &Source::set_max_distance,
            "Maximum distance used by clamped attenuation models.")
        .def_property_readonly("playing", &Source::is_playing,
            "Whether the source is currently playing.")
        .def_property_readonly("paused", &Source::is_paused,
            "Whether the source is currently paused.")
        .def_property_readonly("stopped", &Source::is_stopped,
            "Whether the source is currently stopped.")
        .def_property("position", &Source::get_position, [](Source& s, const std::tuple<float, float, float>& pos) {
            s.set_position(std::get<0>(pos), std::get<1>(pos), std::get<2>(pos));
        }, "3D position (x, y, z).")
        .def_property("velocity", &Source::get_velocity, [](Source& s, const std::tuple<float, float, float>& vel) {
            s.set_velocity(std::get<0>(vel), std::get<1>(vel), std::get<2>(vel));
        }, "3D velocity (x, y, z) for Doppler shift.");

    py::enum_<DistanceModel>(m, "DistanceModel", "Attenuation curve used for distance-based gain rolloff.")
        .value("NONE", DistanceModel::None, "No distance attenuation.")
        .value("INVERSE", DistanceModel::Inverse, "Inverse distance model.")
        .value("INVERSE_CLAMPED", DistanceModel::InverseClamped, "Clamped inverse distance model.")
        .value("LINEAR", DistanceModel::Linear, "Linear distance model.")
        .value("LINEAR_CLAMPED", DistanceModel::LinearClamped, "Clamped linear distance model.")
        .value("EXPONENT", DistanceModel::Exponent, "Exponential distance model.")
        .value("EXPONENT_CLAMPED", DistanceModel::ExponentClamped, "Clamped exponential distance model.")
        .export_values();

    py::class_<Listener>(m, "Listener", "Global 3D listener state.")
        .def_static("reset", &Listener::reset, "Reset listener properties to defaults.")
        .def_property_static("gain", 
            [](py::object) { return Listener::get_gain(); }, 
            [](py::object, float v) { Listener::set_gain(v); },
            "Master listener gain.")
        .def_property_static("distance_model", 
            [](py::object) { return Listener::get_distance_model(); }, 
            [](py::object, DistanceModel v) { Listener::set_distance_model(v); },
            "Active distance attenuation model.")
        .def_property_static("position",
            [](py::object) { return Listener::get_position(); },
            [](py::object, const std::tuple<float, float, float>& pos) {
                Listener::set_position(std::get<0>(pos), std::get<1>(pos), std::get<2>(pos));
            }, "Listener 3D position (x, y, z).")
        .def_property_static("velocity",
            [](py::object) { return Listener::get_velocity(); },
            [](py::object, const std::tuple<float, float, float>& vel) {
                Listener::set_velocity(std::get<0>(vel), std::get<1>(vel), std::get<2>(vel));
            }, "Listener 3D velocity (x, y, z).")
        .def_property_static("orientation",
            [](py::object) { return Listener::get_orientation(); },
            [](py::object, const std::tuple<float, float, float, float, float, float>& ori) {
                Listener::set_orientation(
                    std::get<0>(ori), std::get<1>(ori), std::get<2>(ori),
                    std::get<3>(ori), std::get<4>(ori), std::get<5>(ori)
                );
            }, "Forward and up vectors (f_x, f_y, f_z, up_x, up_y, up_z).");

    py::class_<Stream>(m, "Stream", "Buffered audio streamer for playing files incrementally.")
        .def(py::init<const std::string&, size_t>(), py::arg("path"), py::arg("buffer_size") = 65536,
            "Open an audio file at path for streaming with buffer_size byte chunks.")
        .def("update", &Stream::update,
            "Refill queued stream buffers; call periodically during playback.")
        .def("play", &Stream::play, "Start or resume streaming playback.")
        .def("pause", &Stream::pause, "Pause streaming playback.")
        .def("stop", &Stream::stop, "Stop playback and reset stream position.")
        .def_property_readonly("playing", &Stream::is_playing,
            "Whether the stream is currently playing.")
        .def_property_readonly("paused", &Stream::is_paused,
            "Whether the stream is currently paused.")
        .def_property_readonly("stopped", &Stream::is_stopped,
            "Whether the stream is currently stopped.")
        .def_property("gain", &Stream::get_gain, &Stream::set_gain,
            "Stream gain (volume multiplier).")
        .def_property("pitch", &Stream::get_pitch, &Stream::set_pitch,
            "Pitch multiplier.")
        .def_property("offset", &Stream::get_offset, &Stream::set_offset,
            "Playback offset in seconds.")
        .def_property("looping", &Stream::get_looping, &Stream::set_looping,
            "Whether the stream loops at the end.")
        .def_property("mono", &Stream::get_mono, &Stream::set_mono,
            "Whether mono output is enabled.")
        .def_property_readonly("duration", &Stream::get_total_duration,
            "Total duration in seconds.")
        .def_property_readonly("progress", &Stream::get_progress,
            "Playback progress (typically 0.0 to 1.0).")
        .def_property("position", &Stream::get_position, [](Stream& s, const std::tuple<float, float, float>& pos) {
            s.set_position(std::get<0>(pos), std::get<1>(pos), std::get<2>(pos));
        }, "3D position (x, y, z).")
        .def_property("velocity", &Stream::get_velocity, [](Stream& s, const std::tuple<float, float, float>& vel) {
            s.set_velocity(std::get<0>(vel), std::get<1>(vel), std::get<2>(vel));
        }, "3D velocity (x, y, z).");
}