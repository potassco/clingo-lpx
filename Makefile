CLINGO_DIR:=${HOME}/.local/opt/potassco/debug/lib/cmake/Clingo

all:
	@cmake -G Ninja -B build -DCMAKE_EXPORT_COMPILE_COMMANDS=On -DCLINGOLPX_BUILD_TESTS=On -DClingo_DIR="$(CLINGO_DIR)"
	@cmake --build build
	@compdb -p "build" list -1 > compile_commands.json

test: all
	./build/bin/test_clingo-lpx

clean:
	@cmake --build build --target clean

