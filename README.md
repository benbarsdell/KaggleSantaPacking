Kaggle Santa Packing
====================

C++ code for validating and scoring solutions to the Kaggle holiday competition
[Packing Santa's Sleigh](https://www.kaggle.com/c/packing-santas-sleigh).

About
-----
The code uses the [Thrust](http://thrust.github.io) library and thus can be
compiled for OpenMP, TBB or CUDA backends, making it very fast on multicore
CPUs and Nvidia GPUs.

Methods are provided for efficiently validating and scoring a solution.
The validation routines check that:

- there are the correct number of presents,
- all presents lie within the sleigh boundaries,
- all presents have the correct dimensions, and
- no presents intersect each other.

Building
--------
The code was developed and tested only on an Ubuntu system. Compiling on
other operating systems will likely require some additional work.

The code requires the [Thrust](http://thrust.github.io) header library, and
optionally the [CUDA toolkit](https://developer.nvidia.com/cuda-toolkit) (which
includes a compatible version of Thrust).

To build the code:

> Edit Makefile.inc to match your system configuration

> $ make

To test the code:

> $ make test

Usage
-----
There are two classes:

- SantaProblem, which stores the dimensions of each present, and
- SantaSolution, which stores the min/max coords of each present in a solution,

along with two driver programs:

- check_solution, which reads .csv files and prints out validation and score
information, and
- unit_tests, which performs unit tests on the two classes.

For example:

> $ OMP_NUM_THREADS=4 ./bin/check_solution_omp presents.csv mysubmissionfile.csv

> Load time = 2.78873 s

> Validating solution

> Validation time = 0.518001 s

>                 = 1.9305 Hz

> Evaluating solution

> Evaluation time = 0.080871 s

>                 = 12.3654 Hz

> Solution VERIFIED

> - - - - - - -

> SCORE: 5270836

> - - - - - - -

Contact
-------

Ben Barsdell

benbarsdell at gmail
