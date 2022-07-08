all:
	@cmake -G Ninja -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DCLINGOLPX_BUILD_TESTS=On
	@cmake --build build
	@compdb -p "build" list -1 > compile_commands.json

test: all
	./build/bin/test_clingo-lpx

clean:
	@cmake --build build --target clean

