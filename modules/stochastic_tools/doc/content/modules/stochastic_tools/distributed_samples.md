# Distributed Sampling

Stochastic simulations often require the use of a large number random numbers to compute the
desired calculation. As such it is important to be mindfull of how these numbers are generated
in parallel. Within the stochastic tools module it is possible to generate the sample data in
two modes: replicated or distributed. Each [Sampler](samplers/Sampler.md)-based object has two
methods: `getSamples` and `getLocalSamples`. The former will compute the complete dense matrix and
the later will only compute the portion assigned to the current processor. Sample data is distributed
by row, with each processor being responible for computing a portion of the total number of matrix
rows.

## Example

To demonstate the importance using distributed sample generation a simple example was created using a
test Sampler object that generates 100 million (1e8) random numbers. Each random number generated
within the sample data is 8 bytes, thus in total the sample data requires 800 MB of memory.

The problem was executed in three configurations: (a) a base-line without sample data, (b) complete
sample data on each process, and (c) distributed sample data. Each configuration was executed with
various number of processors from 1 to 16. The total and average per process memory use from each
configuration is shown in [fig:total] and [fig:proc].

As expected, the total memory ([fig:total]) for the non-distributed case increases at a rate of
800 MB per processor and the non-distributed remains constant regardless of the number of
processors utilized. More importantly, the average memory per process decreases in the distributed
configuration.

!media memory_total.svg style=width:100%; id=fig:total
       caption=Total memory usage for three different configurations for sample data generation.

!media memory_per_proc.svg style=width:100%; id=fig:proc
       caption=Average memory per process for three different configurations for sample data generation.
