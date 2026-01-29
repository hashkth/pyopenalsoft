@echo off
setlocal enabledelayedexpansion

:: List of Python versions to build against
set PYTHONS=311 312 313

:: Path prefix to your Python installations
set PYTHON_BASE=C:\Users\kanth\AppData\Local\Programs\Python

:: Make sure release/ and backups/ exist
if not exist release mkdir release
if not exist backups mkdir backups

:: Get timestamp for backup files
for /f %%a in ('powershell -NoProfile -Command "Get-Date -Format \"ddMMyy_HHmmss\""') do set TIMESTAMP=%%a

:: Loop through Python versions
for %%V in (%PYTHONS%) do (
    echo ==============================================
    echo Building for Python %%V
    echo ==============================================

    set "PYTHON_EXE=%PYTHON_BASE%\Python%%V%\python.exe"
    if exist "!PYTHON_EXE!" (
        set "BUILD_DIR=build_%%V"

        cmake -S . -B !BUILD_DIR! ^
            -DCMAKE_BUILD_TYPE=Release ^
            -DPython3_EXECUTABLE="!PYTHON_EXE!" ^
            -DCMAKE_CXX_FLAGS_RELEASE="/O2 /Ob3 /GL /arch:SSE2 /fp:precise" ^
            -DCMAKE_SHARED_LINKER_FLAGS_RELEASE="/LTCG /OPT:REF /OPT:ICF"

        cmake --build !BUILD_DIR! --config Release

        if exist "!BUILD_DIR!\Release\pyopenalsoft.pyd" (
            echo Copying pyopenalsoft%%V.pyd to release folder...
            copy /Y "!BUILD_DIR!\Release\pyopenalsoft.pyd" "release\pyopenalsoft%%V.pyd"

            echo Creating backup pyopenalsoft%%V_!TIMESTAMP!.pyd in backups...
            copy /Y "!BUILD_DIR!\Release\pyopenalsoft.pyd" "backups\pyopenalsoft%%V_!TIMESTAMP!.pyd"
        ) else (
            echo [ERROR] Build for Python %%V failed — no .pyd found.
        )
    ) else (
        echo [ERROR] Python %%V not found at !PYTHON_EXE!
    )
)

echo ==============================================
echo All builds finished!
echo Output in release\pyopenalsoft*.pyd
echo ==============================================

exit /b
