# Distributed Sampling

Stochastic simulations often require the use of a large number random numbers to compute the
desired calculation. As such it is important to be mindfull of how these numbers are generated
in parallel. Within the stochastic tools module it is possible to generate the sample data in
thre modes: replicated, distributed, or iterative. Each [Sampler](samplers/Sampler.md)-based object
has three methods: `getSamples`, `getLocalSamples`, and `getNextLocalRow`. The first will compute
a complete dense matrix, the second will only compute the portion of the matrix assigned to the
current processor, and the last will compute a single row of data within an iterative loop. Sample
data is distributed by row, with each processor being responsible for computing a portion of the
total number of matrix rows.

## Example

To demonstrate the importance of using distributed sample generation a simple example was created
using a test Sampler object that generates 100 million (1e8) random numbers. Each random number
generated within the sample data is 8 bytes, thus in total the sample data requires 800MB of memory.

The problem was executed in four configurations: (a) a base-line without sample data, (b) complete
sample data on each process, (c) distributed sample data, and (d) using the iterative row retrieval.
Each configuration was executed with various number of processors from 1 to 32. The total and average
per process memory use from each configuration is shown in [fig:total] and [fig:proc].

As expected, the total memory ([fig:total]) for the non-distributed case increases at a rate of
800 MB per processor and the non-distributed remains constant regardless of the number of
processors utilized. More importantly, for the distributed case is that, the average memory per
process ([fig:proc]) decreases in the distributed configuration. Obviously, the iterative method performs the
best as only a single row of data exists at any time, thus the memory impact is negligible. For
this reason it is recommend that the `getNextLocalRow` method of sample data retrieval be used
exclusively, unless the the calculation requires the matrix.

!media memory_total.svg style=width:100%; id=fig:total
       caption=Total memory usage for four different configurations for sample data generation.

!media memory_per_proc.svg style=width:100%; id=fig:proc
       caption=Average memory per process for four different configurations for sample data generation.
