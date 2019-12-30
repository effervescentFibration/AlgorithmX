flags = -std=c++17 -Wall
args = src/algorithm_x.cpp src/main.cpp -o bin/algorithm_x

debug_flags = -ggdb -O0 $(flags)
debug_args = $(debug_flags) $(args)

release_flags = -O3 -march=native $(flags)
release_args = $(release_flags) $(args)


release:
	g++ $(release_args)

# run both Clang and GCC to get all warnings.
debug:
	clang++ $(debug_args) && g++ $(debug_args)

clean:
	rm ./bin/algorithm_x
