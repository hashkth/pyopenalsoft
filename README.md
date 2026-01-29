# PyOpenALSoft
Performant OpenALSoft bindings for Python

## Installation
PyOpenALSoft is available on PyPI and can be installed via: `pip install pyopenalsoft`

## Build Instructions
If you wish to build PyOpenALSoft on your own machine, you are free to do so, provided you include the LICENSE with your build when redistributing it.<br>
Ensure that you have installed MSVC compiler and then follow the following steps:
- Make sure you have installed Python 3.11+ on your Windows machine
- Install the latest pybind11 module via `pip install pybind11`
- Open the `pyopenalsoft/` directory through the Visual Studio Developer Command Prompt
- Run build.bat
- Copy `release/pyopenalsoft.pyd` and use it in your projects

## Additional Notes
- Currently, the builds are targeted for Windows 64-bit systems only.
- Linux builds are unavailable.