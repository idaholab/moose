!content pagination previous=tutorial01_app_development/step06_input_params.md
                    next=tutorial01_app_development/step08_test_harness.md
                    margin-bottom=0px

# Step 7: Execute in Parallel

A major objective of MOOSE is performance. This step briefly introduces parallel processing and demonstrates the basic commands used for running a MOOSE application in parallel. A few basic tips on how to evaluate and improve performance are given.

## MOOSE Multiprocessing

There are two types of parallelism supported by MOOSE: [multiprocessing](https://en.wikipedia.org/wiki/Multiprocessing) and [multithreading](https://en.wikipedia.org/wiki/Thread_(computing%29). At its core, MOOSE is designed to run in parallel by using the [Message Passing Interface](https://en.wikipedia.org/wiki/Message_Passing_Interface) protocol. [!ac](MPI) is a library of programming tools for accessing hardware and controlling how multiple CPUs exchange information while working simultaneously to run a single computer program. Shared memory parallelism is also supported through various threading libraries and can be used in union with [!ac](MPI).

The general approach to solving a [!ac](FE) simulation in parallel is to partition the mesh and run an individual process that assembles and solves the system of equations for each of those mesh partitions. In general, the duration the solve procedure decreases as the number of CPUs increases.

### Basic Commands id=commands

The `mpiexec` command is used to execute a MOOSE-based application using [!ac](MPI). For example, the tutorial application can be executed with the following syntax, where the `-n 4` is an argument supplied to the `mpiexec` command that indicates to use 4 processors for execution:

```bash
cd ~/projects/babbler
mpiexec -n 4 ./babbler-opt -i test/tests/kernels/simple_diffusion/simple_diffusion.i
```

In most cases, using [!ac](MPI) alone is the best coarse of action. If desired, threading may
be enabled using the `--n-threads` option, which is supplied directly to the application executable.
For example, the following runs the application with 4 threads:

```bash
cd ~/projects/babbler
./babbler-opt -i test/tests/kernels/simple_diffusion/simple_diffusion.i --n-threads=4
```

It is possible to use both [!ac](MPI) and threading. This is accomplished by combining the two
methods described above.

!alert tip title=Optimum numbers are hardware and problem dependent
The number of processors and threads available for execution is hardware dependent. A modern laptop
typically has 4 processors, with 2 threads each. In general, it is recommended to begin with
using just [!ac](MPI). Thus, it is typical to use between 4 and 8 processors for the `mpiexec`
command. If threading is added, then using 4 processors for [!ac](MPI) and 2 for threading would be
typical. The optimum arrangement for parallel execution will be hardware and problem dependent. It
may be worth while exploring differing arrangements before running a full-scale problem.

!alert note title=Parallel can be enabled in Peacock
In the "Execute" tab of Peacock, the `mpiexec` and `--n-threads` options can be used by selecting the "Use MPI" and "Use Threads" checkboxes and specifying the command syntax. These options can be set and enabled by default in +Peacock > Preferences+.

*For more information about command-line options, please visit the [application_usage/command_line_usage.md] page.*

### Model Setup id=model-setup

The [Mesh] System in MOOSE provides several strategies for configuring a [!ac](FE) model to be solved in parallel. Most end-users won't have to alter the default settings. Even application developers need not worry about writing parallel code, since this is handled by the core systems of MOOSE, [libMesh], and [PETSc]. However, advanced users are likely to encounter situations in which the default parallelization techniques are not suitable for the problem they are solving. Such situations are beyond the scope of this tutorial and interested readers may refer to the following for more information:

- [syntax/Mesh/Partitioner/index.md]
- [syntax/Mesh/index.md#replicated-and-distributed-mesh]
- [syntax/Mesh/splitting.md]
- [source/partitioner/PetscExternalPartitioner.md]


### Evaluating and Enhancing Performance

MOOSE includes a tool for evaluating performance: [PerfGraphOutput.md]. This enables a report to be printed to the terminal that details the amount of time spent processing different parts of the program as well as the total execution time. By evaluating performance reports, the ideal [parallel type](#commands) and [model setup](#model-setup) can be found. This feature can be enabled from the command-line with `--timing` or from within the input file, e.g.,

```
[Outputs]
  perf_graph = true # prints a performance report to the terminal
[]
```

There is an entire field of science about [!ac](HPC) and massively parallel processing. Although it is a valuable one, a formal discussion cannot be made here. One concept worth mentioning is [scalable parallelism](https://en.wikipedia.org/wiki/Scalable_parallelism), which refers to software that performs at the same level for larger problems that use more processes as it does for smaller problems that use fewer processes. In MOOSE, selecting a number of processes based on the number of [!ac](DOFs) in the system is a simple way to try and achieve scalability.

!alert tip title=Try to target 20,000 [!ac](DOFs)-per-process
MOOSE developers tend to agree that 20,000 is the ideal number of [!ac](DOFs) that a single process may be responsible for. This value is reported as "`Num Local DOFs`" in the terminal printout at the beginning of every execution. There are, of course, some exceptions; if a problem exhibits speedup with less than 20,000 [!ac](DOFs)/process, then just use that.

*For more information about application performance, please visit the [application_development/performance_benchmarking.md] page.*

## Demonstration

To demonstrate the importance of parallel execution, the current Darcy pressure input file will be
ran, but with two particular command-line options: First, the performance information shall be
included using the `--timing` option and second, the mesh will be uniformly refined using the `-r`
option to make the problem large enough for analysis.

```bash
cd ~/projects/babbler/problems
./babbler-opt -i pressure_diffusion.i -r 4 --timing
```

!alert warning title=Use less refinement for older hardware
Running this problem with 4 levels of refinement may be too much for older systems. It is still
possible to follow along with this example using less levels of refinement.

The `-r 4` option will split each quadrilateral element into 4 elements, 4 times. Therefore the
resulting mesh will be 4^4^ times larger. The original input file results in 1000 elements, thus
the version executed with this command contains 256,000 elements. This change is evident in the
mesh section of the terminal output. In addition, the number of [!ac](DOFs) is reported, which is
important to consider when selecting the number of processors:

```
Nonlinear System:
  AD size required:        4
  Num DOFs:                257761
  Num Local DOFs:          257761
  Num Partitions:          1
```

The number to consider is the number of local [!ac](DOFs), which is the number of [!ac](DOFs) on
the root processor and is roughly equivalent to the number on the other processors.

The performance information is presented at the end of the simulation, as demonstrated below.

```bash
Performance Graph:
--------------------------------------------------------------------------------------------------------------------------------------------------------------
|                  Section                 | Calls |   Self(s)  |   Avg(s)   |    %   | Children(s) |   Avg(s)   |    %   |  Total(s)  |   Avg(s)   |    %   |
--------------------------------------------------------------------------------------------------------------------------------------------------------------
| BabblerTestApp (main)                    |     1 |      0.006 |      0.006 |   0.04 |      15.048 |     15.048 |  99.96 |     15.054 |     15.054 | 100.00 |
|   FEProblem::outputStep                  |     2 |      0.001 |      0.000 |   0.00 |       0.708 |      0.354 |   4.70 |      0.708 |      0.354 |   4.71 |
|   Steady::PicardSolve                    |     1 |      0.000 |      0.000 |   0.00 |       7.463 |      7.463 |  49.57 |      7.463 |      7.463 |  49.57 |
|     FEProblem::solve                     |     1 |      1.111 |      1.111 |   7.38 |       6.351 |      6.351 |  42.19 |      7.462 |      7.462 |  49.57 |
|       FEProblem::computeResidualInternal |     4 |      0.000 |      0.000 |   0.00 |       1.753 |      0.438 |  11.64 |      1.753 |      0.438 |  11.64 |
|       FEProblem::computeJacobianInternal |     2 |      0.000 |      0.000 |   0.00 |       4.598 |      2.299 |  30.54 |      4.598 |      2.299 |  30.54 |
|     FEProblem::outputStep                |     1 |      0.000 |      0.000 |   0.00 |       0.000 |      0.000 |   0.00 |      0.000 |      0.000 |   0.00 |
|   Steady::final                          |     1 |      0.000 |      0.000 |   0.00 |       0.000 |      0.000 |   0.00 |      0.000 |      0.000 |   0.00 |
|     FEProblem::outputStep                |     1 |      0.000 |      0.000 |   0.00 |       0.000 |      0.000 |   0.00 |      0.000 |      0.000 |   0.00 |
--------------------------------------------------------------------------------------------------------------------------------------------------------------
```

The report indicates that the total duration of the execution was approximately 15 seconds (obviously
this will vary depending on hardware) and the solve time to be approximately 7.5 seconds.

To test the parallel scaling of this [!ac](FE) model it can be executed with an increasing number
of processors. For example, the following executes the same problem with two processors. If the
problem is scalable, then the expectation is that the +solve time+ should be twice as fast.

```bash
cd ~/projects/babbler/problems
mpiexec -n 2 ./babbler-opt -i pressure_diffusion.i -r 4 --timing
```

The data presented in [scale] shows decreasing solve time as the number of processors increases.
This problem was executed on a 2019 Mac Pro with a 2.5 GHz 28-Core Intel Xeon W. For perfect
scaling, the 8-core run should be 8 times faster than the serial execution. Of course, perfect
scaling is not possible due the necessity of performing parallel communication during the solve.
Furthermore, there is a baseline portion of the execution workload that always runs in serial and
does not benefit from parallelism.

!table id=scale caption=Problem solve time with increasing numbers of processors.
| Num. Processors | Local [!ac](DOFs) | Solve Time (sec.) |
| - | - | - |
| 1 | 257,761 | 7.5 |
| 2 | 128,968 | 4.0 |
| 4 |  64,575 | 2.1 |
| 8 |  32,382 | 1.2 |

To be clear, the mesh for the `pressure_diffusion.i` problem was refined here solely for the purpose
of demonstrating how large problems benefit from parallel processing. However, the unrefined mesh
produces a relatively small problem. In practice, a single process is sufficient for any
MOOSE [!ac](FE) problem involving less than 20,000 total [!ac](DOFs).

!content pagination previous=tutorial01_app_development/step06_input_params.md
                    next=tutorial01_app_development/step08_test_harness.md
