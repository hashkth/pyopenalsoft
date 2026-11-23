from typing import Optional, Tuple, List
from enum import Enum


def init(path: Optional[str] = None) -> bool:
    """Load OpenAL Soft from `path` (or default search path if omitted)."""
    ...


def shutdown() -> None:
    """Unload the OpenAL Soft library and reset loader state."""
    ...


def get_dll_path() -> str:
    """Return the path of the loaded OpenAL Soft library."""
    ...


class Device:
    """Audio output device handle."""

    def __init__(self, name: str = "") -> None:
        """Open an audio device by name, or the default device if empty."""
        ...

    @property
    def is_connected(self) -> bool:
        """Whether the device is currently connected."""
        ...

    def reopen(self, name: str = "") -> None:
        """Reopen the device with `name`, or the default device if empty."""
        ...

    def list_devices(self) -> List[str]:
        """Return available audio device names."""
        ...


class Context:
    """OpenAL rendering context bound to a `Device`."""

    def __init__(self, device: Device) -> None:
        """Create and activate a context for `device`."""
        ...


class AudioData:
    """Metadata and PCM decoder for WAV, MP3, and OGG files."""

    def __init__(self, path: str, mono: bool = False) -> None:
        """Load audio metadata from `path`, optionally downmixing to mono."""
        ...

    @property
    def sample_rate(self) -> int:
        """Sample rate in Hz."""
        ...

    @property
    def channels(self) -> int:
        """Number of channels (1 if mono is enabled)."""
        ...

    @property
    def bits_per_sample(self) -> int:
        """Bits per sample."""
        ...

    @property
    def bytes_per_sample(self) -> int:
        """Bytes per sample."""
        ...

    @property
    def total_frames(self) -> int:
        """Total number of sample frames."""
        ...

    @property
    def total_samples(self) -> int:
        """Total number of samples across all channels."""
        ...

    @property
    def duration(self) -> float:
        """Duration in seconds."""
        ...

    @property
    def path(self) -> str:
        """Source file path."""
        ...

    @property
    def mono(self) -> bool:
        """Whether mono downmixing is enabled."""
        ...

    def decode(self) -> bytes:
        """Decode the audio file into raw 16-bit signed PCM bytes."""
        ...


class Buffer:
    """OpenAL buffer holding decoded audio data."""

    def __init__(self, audio_data: AudioData) -> None:
        """Decode `audio_data` and upload it into a new OpenAL buffer."""
        ...


class DistanceModel(Enum):
    """Attenuation curve used for distance-based gain rolloff."""

    NONE: int
    INVERSE: int
    INVERSE_CLAMPED: int
    LINEAR: int
    LINEAR_CLAMPED: int
    EXPONENT: int
    EXPONENT_CLAMPED: int


class Source:
    """3D audio emitter that plays a `Buffer`."""

    def __init__(self) -> None:
        """Create an audio source."""
        ...

    def play(self) -> None:
        """Start or resume playback."""
        ...

    def pause(self) -> None:
        """Pause playback."""
        ...

    def stop(self) -> None:
        """Stop playback and rewind to the start."""
        ...

    def reset(self) -> None:
        """Reset source state and properties."""
        ...

    @property
    def buffer(self) -> Buffer:
        """Attached audio buffer."""
        ...

    @buffer.setter
    def buffer(self, val: Buffer) -> None: ...

    @property
    def gain(self) -> float:
        """Source gain (volume multiplier)."""
        ...

    @gain.setter
    def gain(self, val: float) -> None: ...

    @property
    def pitch(self) -> float:
        """Pitch multiplier."""
        ...

    @pitch.setter
    def pitch(self, val: float) -> None: ...

    @property
    def offset(self) -> float:
        """Playback position in seconds."""
        ...

    @offset.setter
    def offset(self, val: float) -> None: ...

    @property
    def looping(self) -> bool:
        """Whether playback loops."""
        ...

    @looping.setter
    def looping(self, val: bool) -> None: ...

    @property
    def relative(self) -> bool:
        """Whether position and velocity are relative to the listener."""
        ...

    @relative.setter
    def relative(self, val: bool) -> None: ...

    @property
    def reference_distance(self) -> float:
        """Distance at which attenuation begins."""
        ...

    @reference_distance.setter
    def reference_distance(self, val: float) -> None: ...

    @property
    def rolloff_factor(self) -> float:
        """Distance attenuation strength."""
        ...

    @rolloff_factor.setter
    def rolloff_factor(self, val: float) -> None: ...

    @property
    def max_distance(self) -> float:
        """Maximum distance used by clamped attenuation models."""
        ...

    @max_distance.setter
    def max_distance(self, val: float) -> None: ...

    @property
    def playing(self) -> bool:
        """Whether the source is currently playing."""
        ...

    @property
    def paused(self) -> bool:
        """Whether the source is currently paused."""
        ...

    @property
    def stopped(self) -> bool:
        """Whether the source is currently stopped."""
        ...

    @property
    def position(self) -> Tuple[float, float, float]:
        """3D position `(x, y, z)`."""
        ...

    @position.setter
    def position(self, val: Tuple[float, float, float]) -> None: ...

    @property
    def velocity(self) -> Tuple[float, float, float]:
        """3D velocity `(x, y, z)` for Doppler shift."""
        ...

    @velocity.setter
    def velocity(self, val: Tuple[float, float, float]) -> None: ...


class Listener:
    """Global 3D listener state."""

    @staticmethod
    def reset() -> None:
        """Reset listener properties to defaults."""
        ...

    gain: float
    """Master listener gain."""

    distance_model: DistanceModel
    """Active distance attenuation model."""

    position: Tuple[float, float, float]
    """Listener 3D position `(x, y, z)`."""

    velocity: Tuple[float, float, float]
    """Listener 3D velocity `(x, y, z)`."""

    orientation: Tuple[float, float, float, float, float, float]
    """Forward and up vectors `(f_x, f_y, f_z, up_x, up_y, up_z)`."""


class Stream:
    """Buffered audio streamer for playing files incrementally."""

    def __init__(self, path: str, buffer_size: int = 65536) -> None:
        """Open an audio file at `path` for streaming with `buffer_size` byte chunks."""
        ...

    def update(self) -> None:
        """Refill queued stream buffers; call periodically during playback."""
        ...

    def play(self) -> None:
        """Start or resume streaming playback."""
        ...

    def pause(self) -> None:
        """Pause streaming playback."""
        ...

    def stop(self) -> None:
        """Stop playback and reset stream position."""
        ...

    @property
    def playing(self) -> bool:
        """Whether the stream is currently playing."""
        ...

    @property
    def paused(self) -> bool:
        """Whether the stream is currently paused."""
        ...

    @property
    def stopped(self) -> bool:
        """Whether the stream is currently stopped."""
        ...

    @property
    def gain(self) -> float:
        """Stream gain (volume multiplier)."""
        ...

    @gain.setter
    def gain(self, val: float) -> None: ...

    @property
    def pitch(self) -> float:
        """Pitch multiplier."""
        ...

    @pitch.setter
    def pitch(self, val: float) -> None: ...

    @property
    def offset(self) -> float:
        """Playback offset in seconds."""
        ...

    @offset.setter
    def offset(self, val: float) -> None: ...

    @property
    def looping(self) -> bool:
        """Whether the stream loops at the end."""
        ...

    @looping.setter
    def looping(self, val: bool) -> None: ...

    @property
    def mono(self) -> bool:
        """Whether mono output is enabled."""
        ...

    @mono.setter
    def mono(self, val: bool) -> None: ...

    @property
    def duration(self) -> float:
        """Total duration in seconds."""
        ...

    @property
    def progress(self) -> float:
        """Playback progress (typically `0.0` to `1.0`)."""
        ...

    @property
    def position(self) -> Tuple[float, float, float]:
        """3D position `(x, y, z)`."""
        ...

    @position.setter
    def position(self, val: Tuple[float, float, float]) -> None: ...

    @property
    def velocity(self) -> Tuple[float, float, float]:
        """3D velocity `(x, y, z)`."""
        ...

    @velocity.setter
    def velocity(self, val: Tuple[float, float, float]) -> None: ...