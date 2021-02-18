all:
	@cmake -G Ninja -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=On
	@cmake --build build
	@compdb -p "build" list -1 > compile_commands.json

test: all
	./build/test-clingo-lpx

clean:
	@cmake --build build --target clean

