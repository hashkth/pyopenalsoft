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
    m.def("init", [](const std::optional<std::string>& path) -> bool { 
            return OpenALLoader::init(path.value_or("")); 
        }, py::arg("path") = py::none());
    m.def("shutdown", &OpenALLoader::shutdown);
    m.def("get_dll_path", &OpenALLoader::get_dll_path);

    py::class_<Device>(m, "Device")
        .def(py::init<const std::string&>(), py::arg("name") = "")
        .def_property_readonly("is_connected", &Device::is_connected)
        .def("reopen", &Device::reopen, py::arg("name") = "");

    py::class_<Context>(m, "Context")
        .def(py::init<Device&>(), py::keep_alive<1, 2>());

    py::class_<AudioData>(m, "AudioData")
        .def(py::init<const std::string&, bool>(), 
            py::arg("path"), 
            py::arg("mono") = false)
        .def_property_readonly("sample_rate", [](const AudioData& a) { return a.sampleRate; })
        .def_property_readonly("channels", [](const AudioData& a) { return a.channels; })
        .def_property_readonly("bits_per_sample", [](const AudioData& a) { return a.bitsPerSample; })
        .def_property_readonly("bytes_per_sample", [](const AudioData& a) { return a.bytesPerSample; })
        .def_property_readonly("total_frames", [](const AudioData& a) { return a.totalFrames; })
        .def_property_readonly("total_samples", [](const AudioData& a) { return a.totalSamples; })
        .def_property_readonly("duration", [](const AudioData& a) { return a.durationSeconds; })
        .def_property_readonly("path", [](const AudioData& a) { return a.sourcePath; })
        .def_property_readonly("mono", [](const AudioData& a) { return a.forceMono; })
        .def("decode", [](const AudioData& a) {
            auto data = a.decode();
            return py::bytes(reinterpret_cast<const char*>(data.data()), data.size() * sizeof(int16_t));
        });

    py::class_<Buffer>(m, "Buffer")
        .def(py::init<const AudioData&>());

    py::class_<Source>(m, "Source")
        .def(py::init<>())
        .def("play", &Source::play)
        .def("pause", &Source::pause)
        .def("stop", &Source::stop)
        .def("reset", &Source::reset)
        .def_property("buffer", &Source::get_buffer, &Source::set_buffer)
        .def_property("gain", &Source::get_gain, &Source::set_gain)
        .def_property("pitch", &Source::get_pitch, &Source::set_pitch)
        .def_property("offset", &Source::get_offset, &Source::set_offset)
        .def_property("looping", &Source::get_looping, &Source::set_looping)
        .def_property("relative", &Source::get_relative, &Source::set_relative)
        .def_property("reference_distance", &Source::get_reference_distance, &Source::set_reference_distance)
        .def_property("rolloff_factor", &Source::get_rolloff_factor, &Source::set_rolloff_factor)
        .def_property("max_distance", &Source::get_max_distance, &Source::set_max_distance)
        .def_property_readonly("playing", &Source::is_playing)
        .def_property_readonly("paused", &Source::is_paused)
        .def_property_readonly("stopped", &Source::is_stopped)
        .def_property("position", &Source::get_position, [](Source& s, const std::tuple<float, float, float>& pos) {
            s.set_position(std::get<0>(pos), std::get<1>(pos), std::get<2>(pos));
        })
        .def_property("velocity", &Source::get_velocity, [](Source& s, const std::tuple<float, float, float>& vel) {
            s.set_velocity(std::get<0>(vel), std::get<1>(vel), std::get<2>(vel));
        });

    py::enum_<DistanceModel>(m, "DistanceModel")
        .value("NONE", DistanceModel::None)
        .value("INVERSE", DistanceModel::Inverse)
        .value("INVERSE_CLAMPED", DistanceModel::InverseClamped)
        .value("LINEAR", DistanceModel::Linear)
        .value("LINEAR_CLAMPED", DistanceModel::LinearClamped)
        .value("EXPONENT", DistanceModel::Exponent)
        .value("EXPONENT_CLAMPED", DistanceModel::ExponentClamped)
        .export_values();

    py::class_<Listener>(m, "Listener")
        .def_static("reset", &Listener::reset)
        .def_property_static("gain", 
            [](py::object) { return Listener::get_gain(); }, 
            [](py::object, float v) { Listener::set_gain(v); })
        .def_property_static("distance_model", 
            [](py::object) { return Listener::get_distance_model(); }, 
            [](py::object, DistanceModel v) { Listener::set_distance_model(v); })
        .def_property_static("position",
            [](py::object) { return Listener::get_position(); },
            [](py::object, const std::tuple<float, float, float>& pos) {
                Listener::set_position(std::get<0>(pos), std::get<1>(pos), std::get<2>(pos));
            })
        .def_property_static("velocity",
            [](py::object) { return Listener::get_velocity(); },
            [](py::object, const std::tuple<float, float, float>& vel) {
                Listener::set_velocity(std::get<0>(vel), std::get<1>(vel), std::get<2>(vel));
            })
        .def_property_static("orientation",
            [](py::object) { return Listener::get_orientation(); },
            [](py::object, const std::tuple<float, float, float, float, float, float>& ori) {
                Listener::set_orientation(
                    std::get<0>(ori), std::get<1>(ori), std::get<2>(ori),
                    std::get<3>(ori), std::get<4>(ori), std::get<5>(ori)
                );
            });

    py::class_<Stream>(m, "Stream")
        .def(py::init<const std::string&, size_t>(), py::arg("path"), py::arg("buffer_size") = 65536)
        .def("update", &Stream::update)
        .def("play", &Stream::play)
        .def("pause", &Stream::pause)
        .def("stop", &Stream::stop)
        .def_property_readonly("playing", &Stream::is_playing)
        .def_property_readonly("paused", &Stream::is_paused)
        .def_property_readonly("stopped", &Stream::is_stopped)
        .def_property("gain", &Stream::get_gain, &Stream::set_gain)
        .def_property("pitch", &Stream::get_pitch, &Stream::set_pitch)
        .def_property("offset", &Stream::get_offset, &Stream::set_offset)
        .def_property("looping", &Stream::get_looping, &Stream::set_looping)
        .def_property("mono", &Stream::get_mono, &Stream::set_mono)
        .def_property_readonly("duration", &Stream::get_total_duration)
        .def_property_readonly("progress", &Stream::get_progress)
        .def_property("position", &Stream::get_position, [](Stream& s, const std::tuple<float, float, float>& pos) {
            s.set_position(std::get<0>(pos), std::get<1>(pos), std::get<2>(pos));
        })
        .def_property("velocity", &Stream::get_velocity, [](Stream& s, const std::tuple<float, float, float>& vel) {
            s.set_velocity(std::get<0>(vel), std::get<1>(vel), std::get<2>(vel));
        });
}