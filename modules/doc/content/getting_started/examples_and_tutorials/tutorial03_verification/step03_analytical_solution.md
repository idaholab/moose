# Problem with Analytical Solution

## Introduction

The best method for verifying a simulation is comparison with a known mathematical solution.
For the heat equation, as defined in [step02_fem_convergence.md], one such solution
shall be considered to verify a model. The verification process shall be broken into four parts:

1. Define a problem, with a known exact solution.
2. Simulate the problem.
3. Compute the error between the known solution and the simulation.
4. Perform convergence study.

## Problem Statement

[!cite](incropera1996fundamentals) presents a one-dimensional solution for the heat equation as:

!equation id=tutorial03-analytical-solution
T(t,x) = T_0 + \frac{\dot{q}}{k} \Bigg\lbrack 2 \sqrt{\frac{\alpha t}{\pi}}
         \exp\bigg\lparen{\frac{-x^2}{4 \alpha t}}\bigg\rparen -
         x \bigg\lparen 1-\textrm{erf}\bigg\lparen\frac{x}{2\sqrt{\alpha t}} \bigg\rparen \bigg\rparen \Bigg\rbrack,

where $T$ is the temperature, $T_0$ is the initial temperature, $t$ is time, $x$ is position,
$\dot{q}$ is the heat source, $k$ is thermal conductivity, and $\alpha$ is the thermal diffusivity.

Thermal diffusivity is defined as the ratio of thermal conductivity ($k$) to the product of density ($\rho$)
and specific heat capacity ($c$). The system is subjected to Neumann boundary conditions at the
left boundary ($x=0$) and the natural boundary condition (zero flux) on
the opposite boundary.

## Simulation

The heat conduction module of [!ac](MOOSE) is capable of performing a simulation of the above problem
without modification. The values presented in [tutorial03-analytical-values] were selected for the
simulation and a duration of 1 second shall be utilized.

!table id=tutorial03-analytical-values caption=Numerical values used to the solution values of [tutorial03-analytical-solution].
| Variable     | Value | Units |
| :-           | :-    | :-    |
| $T_0$        | 300   | $K$     |
| $\dot{q}$    | 7E5   | $W\cdot m^{-3}$ |
| $k$          | 80.2  | $W\cdot m^{-1}\cdot K^{-1}$ |
| $\rho$       | 7800  | $kg\cdot m^{-3}$ |
| $c$          | 450   | $J\cdot kg^{-1}\cdot K^{-1}$ |
| $q_k(0,t)$   | 7E5   | $W\cdot m^{-2}$ |
| $q_k(0.03,t)$ | 0    | $W\cdot m^{-2}$ |

### Simulation Setup

The following instructions assumes that there is a working version of [!ac](MOOSE) installed
on the system, for information on getting it setup please refer to the
[install instructions](getting_started/installation/index.md).

The simulation to be performed relies on the heat conduction module, thus it is necessary to compile
that specific module. It is assumed that [!ac](MOOSE) was cloned into a directory named "projects"
within the home directory. With this assumption, the following commands will compile and test
the heat conduction module executable.

```bash
cd ~/projects/moose/modules/heat_conduction
make -j8
./run_tests -j8
```

The number used after the "-j" should be the number of processes available on your system. If tests
execute without failure, then the following sections should be followed to setup the simulation.

### Simulation Input

[!ac](MOOSE) operates using input files, as such, an input file is detailed below that is capable of
simulating this problem and its use demonstrated. This section will describe the various sections
of the input file that should be created for the current problem. The location of this input
file is arbitrary, but it is assumed that the content below will be in a file named
`~/projects/problems/verification/1d_analytical.i`. This file can be created and edited
using any text editor program.

Within this file, the domain of the problem is defined using the [Mesh System](syntax/Mesh/index.md)
using the `[Mesh]` block of the input file, as follows, which creates a one-dimensional mesh in the
$x$-direction from 0 to 0.03 $m$ using 200 elements.

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Mesh

There is a single unknown, temperature ($T$), to compute. This unknown is declared using the
[Variables System](syntax/Variables/index.md) in the `[Variables]` block and used the default
configuration of a first-order Lagrange finite element variable. Note, within this block the name of
the variable is arbitrary; "T" was selected here to match the equation.

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Variables

The "volumetric" portions of the weak form are defined using the
[Kernel System](syntax/Kernels/index.md) in the `[Kernels]` block, for this example this
can be done with the use of two `Kernel` objects as follows.

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Kernels

The sub-block "T_time" defines the time derivative and "T_cond" defines the conduction portion
of the equation, please refer to [tutorial03_verification/step01_heat_conduction.md] for details
regarding the weak form of the heat equation. Both blocks include the "variable" parameter, which
is set equal to "T". The remaining parameters define the values for $k$, $\rho$, and $c_p$ as the
constant values defined in [tutorial03-analytical-values].

The boundary portions of the weak form are defined using the
[Boundary Condition System](syntax/BCs/index.md) in the `[BCs]` block. At $x=0$ a
Neumann condition is applied with a value of 7E5. The generated mesh, by default, labels the
boundary at $x=0$ as "left".

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=BCs

The natural boundary condition does not require definition, as the name suggests, this condition is
applied naturally based on the weak form derivation.

The problem is solved using Newton's method with a second-order backward difference formula (BDF2)
with a timestep of 0.01 seconds up to a simulation time of one second. These settings are applied
within the `[Executioner]` block using the [Executioner System](syntax/Executioner/index.md).

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Executioner

Finally, the output method is defined. In this case the ExodusII and CSV format are enabled within the
`[Outputs]` block using the [Outputs System](syntax/Outputs/index.md).

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Outputs

### Simulation Execution

Executing the simulation is straightforward, simply execute the heat conduction module executable
with the input file included using the "-i" option as follows.

```
cd ~/projects/moose/tutorials/tutorial03_verification/step03_analytical
~/projects/moose/modules/heat_conduction/heat_conduction-opt -i 1d_analytical.i
```

When complete an output file will be produced with the name "1d_analytical_out.e", this file
can be viewed using [Paraview](https://www.paraview.org/) or [python/peacock.md].


## Compute Error

To be able to compute the error with respect to the known solution, [tutorial03-analytical-solution]
must be added to the input file. This is accomplished via a [functions/MooseParsedFunction.md] object
that is added in the `[Functions]` block.

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Functions

The "vars" and "vals" have a one-to-one relationship, e.g., $k=80.2$. If defined in this manner
then the names in "vars" can be used in the definition of the equation in the "value" parameter.

The error is a single value for each time step. Since the exact solution is known the $L_2$-norm
can be computed using the [postprocessors/NodalL2Error.md] object within the `[Postprocessors]` block,
which computes the error of the given variable (e.g., "T") and the known solution defined in the
"T_exact" function.

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Postprocessors

In addition to the computed error, the average element size is also output. As detailed
previously, this quantity is necessary to perform the convergence study.

Another useful item to add to the input, especially for a one-dimensional problem, are line samples
along the length of the simulation for the computed and exact solution via the
`[VectorPostprocessors]` block.

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=VectorPostprocessors

In this block the both the exact solution function ("T_exact") and the finite element solution ("T")
are sampled along the length of the simulation. The $L_2$-norm as well as these line sampled values
are both output as CSV files for further analysis.

[tutorial03-simulation-results] shows the complete results, including the simulated solution,
the exact solution, and the absolute error as a function of simulation time.

!media tutorial03_verification/1d_analytical.mp4 id=tutorial03-simulation-results
       caption=Comparison of simulation results, the exact solution, and the computed absolute error
               for the one-dimensional heat equation.


## Convergence Studies

As shown in [tutorial03-simulation-results] the results obtained from the simulation seem to
agree well with the known solution. The results provide evidence that the finite element
calculations are being performed correctly. As is, the error shown could be due to a problem
with the finite element implementation. However, if the error is reduced at the expected rate
with decreasing mesh size and timestep size then it is reasonable that the finite element
formulation and temporal numerical integration are implemented correctly.

[!ac](MOOSE) includes a python package---the `mms` package---for performing convergence studies, which will be used here. For details regarding the package, please refer to the [python/mms.md] documentation.

### Spatial Convergence

To perform a spatial convergence the input file created needs to be executed with decreasing element
size, which in [!ac](MOOSE) can be done by adding the `Mesh/uniform_refine=x` option to the
command line with "x" being an integer representing the number of refinements to perform.

The `mms` package includes a function (`run_spatial`) for performing and automatically modifying
the supplied input file to perform increasing levels of uniform refinement. After each run the
results are collected into a `pandas.DataFrame` object, which can be used in the `ConvergencePlot`
object to create a plot.

For example, for [tutorial03-analytical-spatial-code] results of the spatial convergence study are shown as a plot in
[tutorial03-analytical-spatial-graph]. In this case, the input file created above (`1d_analytical.i`)
is executed with six levels of uniform refinement (0--5). Notice that an extra
argument `Mesh/nx=10` is supplied to reduce the number of elements to 10 for the initial mesh. The
`console` flag suppress the screen output from the simulations.

!listing tutorial03_verification/app/test/tests/step03_analytical/step03_study.py id=tutorial03-analytical-spatial-code
         caption=Example use of the `mms` package to run and plot spatial convergence.
         link=False start=MooseDocs:start:spatial end=MooseDocs:end:spatial include-start=False

The resulting convergence data indicate that the simulation is converging to the known solution
at the correct rate for first-order elements, thus verifying that the finite element solution
is computing the results as expected.

!media tutorial03_verification/1d_analytical_spatial.png
       id=tutorial03-analytical-spatial-graph
       caption=Results of the spatial convergence study comparing the finite element simulation with the known one-dimensional analytical solution.

### Temporal Convergence

The temporal convergence study is nearly identical to the spatial, as shown in
[tutorial03-analytical-temporal-code]. For a temporal study, the `run_temporal` is used and the
initial time step is set using `dt`; the number of elements is not altered. The resulting
converge graph is shown in [tutorial03-analytical-temporal-graph].

!listing tutorial03_verification/app/test/tests/step03_analytical/step03_study.py id=tutorial03-analytical-temporal-code
         caption=Example use of the `mms` package to run and plot temporal convergence.
         link=False start=MooseDocs:start:temporal end=MooseDocs:end:temporal include-start=False

The resulting convergence data indicate that the simulation is converging to the known solution
at the correct rate for second-order time integration, thus verifying that the numerical integration
scheme is operating as expected.

!media tutorial03_verification/1d_analytical_temporal.png
       id=tutorial03-analytical-temporal-graph
       caption=Results of the spatial convergence study comparing the finite element simulation with the known one-dimensional analytical solution.

## Closing Remarks

The analysis performed here provides three levels of verification: (1) the finite element solution
matches the known solution with minimal error, (2) the spatial error converges at the
expected rate for first-order finite elements, and (3) the temporal error converges at the
expected rate for second-order numerical integration. These results verify that the
simulation is correctly solving the 1D heat equation.

!bibtex bibliography !!include to make sure next/previous are last on page

!content pagination previous=tutorial03_verification/step02_fem_convergence.md
                    next=tutorial03_verification/step04_mms.md
