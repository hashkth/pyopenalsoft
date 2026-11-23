# PyOpenALSoft

Performant OpenAL Soft bindings for Python.

## Installation

PyOpenALSoft is available on [PyPI](https://pypi.org/project/pyopenalsoft/) and can be installed using pip:

```bash
pip install pyopenalsoft
```

## Documentation

For detailed documentation, usage examples, API references, and additional information, visit the [PyOpenALSoft Wiki](../../wiki).

## Build Instructions

You can build PyOpenALSoft locally, provided that you include the `LICENSE` file when redistributing your build.

### Prerequisites

Before proceeding, ensure that you have the following installed:

- Python 3.11 or later (excluding Python 3.15)
- MinGW-w64 G++ compiler
- Latest version of `pybind11`

Install `pybind11` using:

```bash
pip install pybind11
```

### Building

1. Open a terminal in the `pyopenalsoft/` directory.
2. Run the following command to build the library:

   ```bash
   make VER=3.1X
   ```

3. Once the build completes, locate the generated file:

   ```text
   release/pyopenalsoft31x.pyd
   ```

4. Copy the `.pyd` file into your project and import it as needed.

## Additional Notes

- Currently, builds target **Windows 64-bit systems only**.
- Linux builds are not currently supported.
