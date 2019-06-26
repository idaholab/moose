# Memory and time scaling behavior of Kuzmin-Turek stabilization

This page is part of a set of pages devoted to discussions of numerical stabilization in PorousFlow.  See:

- [Numerical stabilization lead page](stabilization.md)
- [Mass lumping](mass_lumping.md)
- [Full upwinding](upwinding.md)
- [Kuzmin-Turek stabilization](kt.md)
- [Numerical diffusion](numerical_diffusion.md)
- [A worked example of Kuzmin-Turek stabilization](kt_worked.md)
- [Memory and cpu-time scaling of Kuzmin-Turek stabilization](kt_scaling_study.md)

Because Kuzmin-Turek stabilization requires information about next-nearest nodes, it does not fit into the usual finite-element scheme used by MOOSE, and it is more computationally and memory intensive.  This page quantifies the memory and compute-time used in Kuzmin-Turek stabilization, concentrating on quantifying the costs incurred when computing the extra terms needed in KT stabilization.

It is well-known that Kuzmin-Turek stabilization yields a more difficult nonlinear problem than when using full upwinding or no stabilization.  It is rather complicated to quantify this difference between KT and full-upwinding and no stabilization, however, because it is problem-dependent.  Therefore, these additional nonlinearities are not considered in this page: only the memory and cpu-time costs associated with computing the extra terms needed in KT stabilization are quantified.  To side-step this issue, all simulations use the command-line arguments `Executioner/solve_type=Linear Executioner/l_tol=0.999` to ensure that every time-step involves just one linear solve and zero nonlinear iterations.  This means the results of the simulations aren't physically correct, of course, but this does not impact the scaling behavior that is studied here.

The results quoted on this page were generated using a dedicated node with 20 cores on the CSIRO Pearcey cluster.

## Summary of findings for 1D convection

As discussed above, the following points do not include any impact from the likely increase in nonlinearity when using Kuzmin-Turek stabilization, and the consequent increase in number of nonlinear iterations.  For convection in 1D, the results below demonstrate that:

- Using both MOOSE framework objects and PorousFlow (Kuzmin-Turek) objects, the physical memory requirements and the virtual memory requirements scale as 1/number_of_processors.  The memory required in the first time step is approximately 10 times less than subsequent time steps.
- Using Kuzmin-Turek stabilization uses approximately 10% more memory than convection using MOOSE framework (unstabilized) objects.
- Compared with subsequent time steps, the first time step requires about 70 times more compute time when using MOOSE framework objects, and about 300 times more compute time when using Kuzmin-Turek objects.
- In the worst case scenario, the first time step requires almost 100 times more compute time for Kuzmin-Turek stabilization than when using MOOSE framework objects.
- In subsequent time steps, Kuzmin-Turek stabilization is approximately 20 times slower than when using MOOSE framework objects.

## 1D Framework benchmark case

The [1D convection simulation](https://github.com/idaholab/moose/blob/master/modules/porous_flow/test/tests/numerical_diffusion/framework.i) is used as a benchmark.  The quantities of interest are:

- how the memory scales with number of processors used;
- how the cpu-time required scales with number of processors used.  The CPU-time involves just one $Ax=b$ linear solve and no nonlinear iterations.

It is run using the command

```
mpirun -np N porous_flow-opt Mesh/type=DistributedGeneratedMesh Mesh/nx=M Outputs/perf/execute_on=TIMESTEP_END Executioner/end_time=18E-1 Executioner/solve_type=Linear Executioner/l_tol=0.999 --distributed_mesh -i framework.i
```

Here `N` is the number of processors used, and `M` is the number of elements in the mesh.  The values used are

- `N` runs over 1, 2, 4, 8 and 16.
- `M` runs over 100, 1000, 2000, 4000, 8000, 16000, 32000, 64000, 128000 and 256000

The other command-line arguments indicate:

- use a distributed mesh (so each processor doesn't know about the entire mesh);
- output results every time-step;
- use just 3 time-steps (a greater number is unnecessary);
- and use `solve_type=Linear` because the physical convection modelled by this input file is not of particular interest (only the scaling behaviour is of interest here).

### Memory

The physical and virtual memory:

- are fairly low in the first time step
- increase by up to approximately a factor of 10 for the second time step
- increase very slightly for every subsequent time-step.

When the mesh contains only `M=100` elements, the memory used is essentially just the memory required by the `porous_flow-opt` executable, as shown in [framework_100_mem]

!media numerical_diffusion/framework_100_mem.png style=width:60%;margin-left:10px caption=Memory used by the framework simulation with 100 elements, at the third time step.  id=framework_100_mem

When the mesh contains `M=256000` elements, the memory used is much larger.  Subtracting the memory required by the `porous_flow-opt` executable (assuming this is the memory recorded by the `M=100`) as shown in [framework_256000_mem], and the expected scaling behaviour, ${\mathtt{mem}}\propto 1/{\mathtt{numprocs}}$, is shown in [framework_256000_mem_log]

!media numerical_diffusion/framework_256000_mem.png style=width:60%;margin-left:10px caption=Memory used by the framework simulation with 256000 elements, at the third time step.  id=framework_256000_mem

!media numerical_diffusion/framework_256000_mem_log.png style=width:60%;margin-left:10px caption=Memory used by the framework simulation with 256000 elements (at the third time step) is well-described by the expected power law.  id=framework_256000_mem_log

### CPU time in first time step for `M=256000`

The CPU time taken by the first time step is shown in [framework_1_cpu], while [framework_1_cpu_log] demonstrates reasonable scaling behavior with the number of processors.

!media numerical_diffusion/framework_1_cpu.png style=width:60%;margin-left:10px caption=CPU time required for the first time step by the framework simulation with 256000 elements.  id=framework_1_cpu

!media numerical_diffusion/framework_1_cpu_log.png style=width:60%;margin-left:10px caption=CPU time required for the first time step by the framework simulation with 256000 elements, is well-described by a power law.  id=framework_1_cpu_log

### CPU time in first time step for varying numbers of elements

The CPU time taken by the first time step increases with the number of elements, as shown in [framework_1_cpu_eles] (this is for 1 processor: the same trend is observed for other numbers of processors).

!media numerical_diffusion/framework_1_cpu_eles.png style=width:60%;margin-left:10px caption=CPU time required for the first time step by the framework simulation increases with the number of elements.  id=framework_1_cpu_eles

### CPU time in subsequent time steps for `M=256000`

The CPU time taken by subsequent time steps is shown in [framework_3_cpu], while [framework_3_cpu_log] demonstrates reasonable scaling behavior with the number of processors.  The CPU time required is about 70 times less than the first time step.

!media numerical_diffusion/framework_3_cpu.png style=width:60%;margin-left:10px caption=CPU time required for subsequent time steps by the framework simulation with 256000 elements.  id=framework_3_cpu

!media numerical_diffusion/framework_3_cpu_log.png style=width:60%;margin-left:10px caption=CPU time required for the subsequent time steps by the framework simulation with 256000 elements, is well-described by a power law.  id=framework_3_cpu_log


## 1D Kuzmin-Turek convection

The [1D fully-saturated convection simulation](https://github.com/idaholab/moose/blob/master/modules/porous_flow/test/tests/numerical_diffusion/pffltvd_action.i) is studied in this section.  As above, the quantities of interest are:

- how the memory scales with number of processors used;
- how the cpu-time required scales with number of processors used.  The CPU-time involves just one $Ax=b$ linear solve and no nonlinear iterations.

A similar command is used to run the simulations:

```
mpirun -np N porous_flow-opt Mesh/type=DistributedGeneratedMesh Mesh/nx=M Outputs/perf/execute_on=TIMESTEP_END Executioner/end_time=18E-2 Executioner/solve_type=Linear Executioner/l_tol=0.999 --distributed_mesh -i pffltvd_action.i
```

These arguments are the same as used for the framework benchmark case.

### Memory

The physical and virtual memory:

- are comparitively low in the first time step
- increase by up to approximately a factor of 10 for the second time step
- increase very slightly for every subsequent time-step.

This is qualitatively similar to the framework case.  Just as in the framework case, the memory used with a small number of elements may be used to estimate the size of the `porous_flow-opt` executable, and subtracted from the memory used when the number of elements is large.  The results are shown in [pffltvd_action_100_mem], [pffltvd_action_256000_mem] and [pffltvd_action_256000_mem_log].

The expected scaling behaviour, ${\mathtt{mem}}\propto 1/{\mathtt{numprocs}}$, is followed approximately.  Approximately $(3.0/2.7)-1\approx 10\%$ times more memory is needed compared with the benchmark framework case.

!media numerical_diffusion/pffltvd_action_100_mem.png style=width:60%;margin-left:10px caption=Memory used by the 1D Kuzmin-Turek simulation with 100 elements, at the third time step.  id=pffltvd_action_100_mem

!media numerical_diffusion/pffltvd_action_256000_mem.png style=width:60%;margin-left:10px caption=Memory used by the 1D Kuzmin-Turek simulation with 256000 elements, at the third time step.  id=pffltvd_action_256000_mem

!media numerical_diffusion/pffltvd_action_256000_mem_log.png style=width:60%;margin-left:10px caption=Memory used by the 1D Kuzmin-Turek simulation with 256000 elements (at the third time step) may be compared to a power law.  id=pffltvd_action_256000_mem_log

### CPU time in first time step for `M=256000`

The CPU time taken by the first time step is shown in [pffltvd_action_1_cpu], while [pffltvd_action_1_cpu_log] demonstrates reasonable scaling behavior with the number of processors.

!media numerical_diffusion/pffltvd_action_1_cpu.png style=width:60%;margin-left:10px caption=CPU time required for the first time step by the 1D Kuzmin-Turek simulation with 256000 elements.  id=pffltvd_action_1_cpu

!media numerical_diffusion/pffltvd_action_1_cpu_log.png style=width:60%;margin-left:10px caption=CPU time required for the first time step by the 1D Kuzmin-Turek simulation with 256000 elements, is well-described by a power law.  id=pffltvd_action_1_cpu_log

The CPU time can be about 100 times larger than the benchmark framework case.  Why?  At the moment, I'm not sure.  The heaviest branch is, by far, `NonlinearSystemBase::computeJacobianTags`, which corresponds to the `Threads::parallel_reduce(elem_range, cj);` line in `NonlinearSystemBase.C`.  I put a `TIME_SECTION` in `PorousFlowFluxLimitedTVDAdvection::computeJacobian()` and that is not taking appreciable time.

### CPU time in first time step for varying numbers of elements

The CPU time taken by the first time step increases with the number of elements, as shown in [pffltvd_action_1_cpu_eles], for 1 processor.  Similar scaling behaviour is displayed for different number of processors.

!media numerical_diffusion/pffltvd_action_1_cpu_eles.png style=width:60%;margin-left:10px caption=CPU time required for the first time step by the 1D Kuzmin-Turek simulation depends approximately quadratically on the number of elements.  id=pffltvd_action_1_cpu_eles


### CPU time in subsequent time steps for `M=256000`

The CPU time taken by subsequent time steps is shown in [pffltvd_action_3_cpu], while [pffltvd_action_3_cpu_log] demonstrates reasonable scaling behavior with the number of processors.  The CPU time required is about 200 times less than the first time step, but about 20 times greater than the framework simulation.

!media numerical_diffusion/pffltvd_action_3_cpu.png style=width:60%;margin-left:10px caption=CPU time required for subsequent time steps by the 1D Kuzmin-Turek simulation with 256000 elements.  id=pffltvd_action_3_cpu

!media numerical_diffusion/pffltvd_action_3_cpu_log.png style=width:60%;margin-left:10px caption=CPU time required for the subsequent time steps by the 1D Kuzmin-Turek simulation with 256000 elements, is well-described by a power law.  id=pffltvd_action_3_cpu_log
