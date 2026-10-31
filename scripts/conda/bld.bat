cmake -S . -B build -G Ninja ^
    -DCMAKE_CXX_COMPILER="%CXX%" ^
    -DCMAKE_C_COMPILER="%CC%" ^
    -DCMAKE_BUILD_TYPE=Release ^
    -DCLINGOLPX_MANAGE_RPATH=OFF ^
    -DCLINGOLPX_BUILD_TESTS=ON ^
    -DCMAKE_INSTALL_PREFIX="%LIBRARY_PREFIX%" ^
    -DCMAKE_INSTALL_LIBDIR="lib" ^
    -DPYCLINGOLPX_INSTALL_DIR="%SP_DIR%" ^
    -DPython_FIND_STRATEGY="LOCATION" ^
    -DPython_ROOT_DIR="%PREFIX%" ^
    -DPython_EXECUTABLE="%PYTHON%" || exit /b 1

cmake --build build || exit /b 1
ctest --output-on-failure --test-dir build || exit /b 1
cmake --build build --target install || exit /b 1
