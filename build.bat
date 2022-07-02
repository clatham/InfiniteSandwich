set CWD=%cd%
set PROJECT_DIR=%~dp0.
set CMAKE_TOOL=%PROJECT_DIR%\vcpkg\downloads\tools\cmake-3.22.2-windows\cmake-3.22.2-windows-i386\bin\cmake.exe


cd %PROJECT_DIR%


if not exist vcpkg\vcpkg-bootstrap.bat (
    git submodule update --init --recursive
)


if not exist vcpkg\vcpkg.exe (
    call vcpkg\bootstrap-vcpkg.bat -disableMetrics
)


if not exist %CMAKE_TOOL% (
    vcpkg\vcpkg.exe install --triplet=x64-windows
)


if exist build\ (
    rmdir /s /q build
)


mkdir build
cd build

%CMAKE_TOOL% -DCMAKE_TOOLCHAIN_FILE=%PROJECT_DIR%\vcpkg\scripts\buildsystems\vcpkg.cmake ..
%CMAKE_TOOL% --build . --config Release --target ALL_BUILD


cd %CWD%
