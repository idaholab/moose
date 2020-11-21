# Step 7: Execute in Parallel

One of the major objectives of the MOOSE Framework is high performance. This step briefly introduces parallel processing with MOOSE. The basic commands used for running an application on multiple cores and/or multiple threads-per-core are demonstrated. A few basic tips on how to evaluate and improve performance are given.

## MOOSE Multiprocessing

At its core, MOOSE is designed to run in parallel by using the [Message Passing Interface](https://en.wikipedia.org/wiki/Message_Passing_Interface) protocol. [!ac](MPI) is a library of programming tools for accessing hardware and controlling how multiple CPUs exchange information while working simultaneously to run a single computer program. The general approach to solving a [!ac](FE) simulation in parallel is to partition the mesh and run an individual process that assembles and solves the system of equations for each of those mesh partitions. The hope is that the duration of solve procedures are decreased as the number of CPUs they engage is increased.

### Model Setup id=model-setup

The [Mesh] System in MOOSE provides several strategies for configuring a [!ac](FE) model to be solved in parallel. Most end-users won't have to alter the default settings. Even application developers need not worry about writing parallel code, since this is handled by the core systems of MOOSE, [libMesh], and [PETSc]. However, advanced users are likely to encounter situations in which the default parallelization techniques are not suitable for the problem they are solving. Such situations are beyond the scope of this tutorial and interested readers may refer to the following for more information:

- [syntax/Mesh/Partitioner/index.md]
- [syntax/Mesh/index.md#replicated-and-distributed-mesh]
- [syntax/Mesh/splitting.md]
- [source/partitioner/PetscExternalPartitioner.md]

### Basic Commands id=commands

A parallel execution is invoked at the command line along with the usual syntax for running an input file. There are two types of parallelism: [multiprocessing](https://en.wikipedia.org/wiki/Multiprocessing) and [multithreading](https://en.wikipedia.org/wiki/Thread_(computing%29). The former is enabled by the `mpiexec` command and the latter by the `--n-threads` option. For example, the following commands will run `simple_diffusion.i` on four CPUs, each with two threads:

```bash
cd ~/projects/babbler
mpiexec -n 4 ./babbler-opt -i test/tests/kernels/simple_diffusion/simple_diffusion.i --n-threads=2
```

There are advantages and disadvantages to using multiprocessing, multithreading, or both. In most cases, the highest speedup will come from just using `mpiexec`. However, communicating information between CPUs efficiently is perhaps the most difficult challenge with parallel processing, since each process has a separate memory space. On the other hand, multithreading uses a shared memory space, but threads are not truly *parallel* processes because they run on a single CPU.

!alert note title=Parallel Execution in PEACOCK
In the "Execute" tab of PEACOCK, the `mpiexec` and `--n-threads` options can be used by selecting the "Use MPI" and "Use Threads" checkboxes and specifying the command syntax. These options can be set and enabled by default in the PEACOCK preferences.

*For more information about command-line options, please visit the [application_usage/command_line_usage.md] page.*

### Evaluating and Enhancing Performance

The most basic tool available for evaluating performance is the [source/actions/CommonOutputAction.md] `"perf_graph"` parameter. This enables a report to be printed to the terminal that details the amount of time spent processing different parts of the program as well as the total execution time. By evaluating performance reports, the ideal [model setup](model-setup) and [parallel type](#commands) can be found. This feature can be enabled in an input file like so:

```
[Outputs]
  perf_graph = true
[]
```

There is an entire field of science about [!ac](HPC) and massively parallel processing. Although it is a valuable one, a formal discussion cannot be made here. One concept worth mentioning is [scalable parallelism](https://en.wikipedia.org/wiki/Scalable_parallelism), which refers to software that performs at the same level for larger problems that use more processes as it does for smaller problems that use fewer processes. In MOOSE, selecting a number of processes based on the number of [!ac](DOFs) in the system is a simple way to try and achieve scalability.

!alert tip title=Try to target 20,000 [!ac](DOFs)-per-process
MOOSE developers tend to agree that 20,000 is the ideal number of [!ac](DOFs) that a single process be responsible for. This value is reported as "`Num Local DOFs`" in the terminal printout at the beginning of every execution.

*For more information about application performance, please visit the [application_development/performance_benchmarking.md] page.*

## Demonstration

First, add the `perf_graph = true` input to the `[Outputs]` block in the `pressure_diffusion.i` Darcy pressure input file and run the code in serial:

```bash
cd ~/projects/babbler
./babbler-opt -i problems/pressure_diffusion.i
```

The initial report printed to the terminal indicates that there are 1111 [!ac](DOFs)-per-process, i.e., the printout should contain something like the following:

```bash
Nonlinear System:
  AD size required:        4
  Num DOFs:                1111
  Num Local DOFs:          1111   # number of DOFs on proc 0 roughly equal to the number on all other procs
```

With `perf_graph = true`, the last thing printed to the terminal should be the performance report, which should look something like the following:

```bash
Performance Graph:
--------------------------------------------------------------------------------------------------------------------------------------------------------------
|                  Section                 | Calls |   Self(s)  |   Avg(s)   |    %   | Children(s) |   Avg(s)   |    %   |  Total(s)  |   Avg(s)   |    %   |
--------------------------------------------------------------------------------------------------------------------------------------------------------------
| BabblerTestApp (main)                    |     1 |      0.011 |      0.011 |   8.70 |       0.116 |      0.116 |  91.30 |      0.127 |      0.127 | 100.00 |
|   FEProblem::outputStep                  |     2 |      0.000 |      0.000 |   0.22 |       0.007 |      0.003 |   5.32 |      0.007 |      0.004 |   5.53 |
|   Steady::PicardSolve                    |     1 |      0.000 |      0.000 |   0.04 |       0.065 |      0.065 |  51.27 |      0.065 |      0.065 |  51.31 |
|     FEProblem::solve                     |     1 |      0.007 |      0.007 |   5.63 |       0.058 |      0.058 |  45.47 |      0.065 |      0.065 |  51.10 |
|       FEProblem::computeResidualInternal |     4 |      0.000 |      0.000 |   0.00 |       0.018 |      0.004 |  14.10 |      0.018 |      0.004 |  14.11 |
|       FEProblem::computeJacobianInternal |     2 |      0.000 |      0.000 |   0.00 |       0.040 |      0.020 |  31.25 |      0.040 |      0.020 |  31.26 |
|     FEProblem::outputStep                |     1 |      0.000 |      0.000 |   0.08 |       0.000 |      0.000 |   0.00 |      0.000 |      0.000 |   0.08 |
|   Steady::final                          |     1 |      0.000 |      0.000 |   0.01 |       0.000 |      0.000 |   0.03 |      0.000 |      0.000 |   0.04 |
|     FEProblem::outputStep                |     1 |      0.000 |      0.000 |   0.02 |       0.000 |      0.000 |   0.01 |      0.000 |      0.000 |   0.03 |
--------------------------------------------------------------------------------------------------------------------------------------------------------------
```

The report indicates that the total duration of the execution was approximately 0.127 seconds (obviously this will vary depending on hardware).

To test the parallel scalability of this [!ac](FE) model without modifying the input file, the `-r` command-line option can be used to automatically perform a uniform mesh refinement before solving:

```bash
cd ~/projects/babbler
./babbler-opt -i problems/pressure_diffusion.i -r 1 # splits QUAD elements into four smaller QUADs
```

Now, the initial report should show that there are 4221 [!ac](DOFs)-per-process and the performance report might indicate that the execution took roughly four times as long. Since there are about four times as many DOFs on a single process for the refined mesh as there were for the unrefined, the problem ought to be solved with four processes to obtain a similar level of performance. Thus, run the input file using the following commands:

```bash
cd ~/projects/babbler
mpiexec -n 4 ./babbler-opt -i problems/pressure_diffusion.i -r 1 # run refined mesh on four procs
```

Now the printout should indicate that there are 1077 [!ac](DOFs)-per-process, roughly equal to that of the unrefined mesh ran in serial. The total execution time will be shorter than that of the refined mesh in serial, but certainly not as short as the unrefined in serial. While this might suggest poor scaling, one may now realize that this is why an ideal number of [!ac](DOFs)-per-process (20,000) exists at all. There is simply not enough calculations to perform to for the problem to scale well in parallel. In practice, a single process is sufficient for any MOOSE [!ac](FE) problem that has less than 20,000 total [!ac](DOFs).

!content pagination previous=tutorial01_app_development/step06_input_params.md
                    next=tutorial01_app_development/step08_test_harness.md
