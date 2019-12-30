# Algorithm ✗ 🎲
This repo hosts a straightforward implementation, currently in C++, of Donald Knuth's Algorithm X, which uses the dancing links technique to solve the exact cover problem in a backtracking fashion, as presented in _The Art of Computer Programming_, volume 4, fascicle 5. It's meant to closely follow the text's presentation for pedagogical reasons. This project may evolve to solve related problems, improve performance, provide stronger guarantees, or offer other tooling, or it may do none of these things. 🃏


## Use ♞
The only hard dependency is a C++ compiler that supports C++17.


If you have modern versions of Clang and GCC installed along with Make on a Unix-like system, a debug release can be built in this directory as follows:
```
$ make debug
```
Likewise, a more performant release candidate can be made this way:
```
$ make release
```

In either case, the binary can then be run with:
```
$ bin/algorithm_x
```

To remove the binary, run:
```
$ make clean
```


## Organization 💃
The implementation of algorithm X lives in a single ExactCoverProblem class defined in `./src/algorithm_x.h` and implemented in `./src/algorithm_x.cpp`. The main function is defined in `./src/main.cpp`, which gives a simple example of its use taken from the Knuth book. Attempts are made to use up-to-date C++ coding conventions and make performant choices where appropriate, but no particular standard is followed. Emphasis is on clarity and faithfulness to Knuth's exposition. 


## Caveat emptor 🔗
Everything here is intended just for education. I hope it's right, but it might not be. It might get your queens in a standoff, accidentally route your delivery truck to the Peruvian Amazon, or unfetter the unwholesome forces that C++ tries to balance, depending on what you do with it. It's definitely not warrantied fit for any particular purpose.


December, 2019

Seattle/Kirkland, Washington, USA 🏴󠁵󠁳󠁷󠁡󠁿
