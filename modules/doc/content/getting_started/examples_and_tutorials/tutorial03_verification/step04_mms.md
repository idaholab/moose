# Method of Manufactured Solution

## Introduction

In practice an exact solution to the problem being solved is rarely known, since there would
be no reason to perform a simulation if the answer is known. If the solution
is not known then the error cannot be computed. However, it is possible to "manufacture" a solution
using a method known as the [!ac](MMS). In this step of the tutorial the two-dimensional heat equation
will be used to simulate a physical system. Then using this simulation as a starting point a
spatial and temporal convergence study will be performed using the [!ac](MMS).

## Problem Statement

For this example, the two-dimensional heat equation shall be used to simulate a cross section
of snow subjected to radiative cooling at the surface and internal heating due to solar absorption.

The domain of the problem is defined as $x \in [0, 1 \textrm{m}]$ and $y \in [-0.2 \textrm{m},
0]$. Referring to the heat equation defined in
[Step 1](tutorial03_verification/step01_heat_conduction.md), a heat source ($\dot(q)$) is applied as
defined in [tutorial03_heat_source]. This source is a crude approximation of internal heating due to
solar absorption. The source raised from zero to a maximum of 650 $W\cdot m^{-3}$ back to zero during
a 9 hours (32400 sec.) period following a sine function.  The duration is an approximation of the
daylight hours in Idaho Falls, Idaho in December. Also, the left side of the domain is shaded and the
right is subjected to the maximum source, again defined using a sine function.

!equation id=tutorial03_heat_source
\dot{q} = 650\cdot\sin\bigg(\frac{x \pi}{2}\bigg) \cdot \textrm{exp}(\Kappa y) \cdot \sin\bigg(\frac{\pi t}{32400}\bigg),

where $\Kappa$ is the extinction coefficient.

The system is subjected to Neumann boundary conditions at the top surface boundary ($y=0$),
which approximates the radiative cooling of the snow surface. The left and
right sides of the domain are subjected to the natural boundary condition, which for this problem
approximates an "insulated" boundary condition. Finally, a constant
temperature is imposed on the bottom boundary.

## Simulation

The heat conduction module of [!ac](MOOSE) is capable of performing the desired simulation.
The values in [tutorial03-snow-values] provide the numeric values to be used for the simulation,
which will solve for nine hours of simulation time.

!table id=tutorial03-snow-values caption=Numerical values used for two-dimensional simulation of snow.
| Variable          | Value  | Units |
| :-                | :-     | :-    |
| $T_0$             | 263.15 | $K$     |
| $k$               | 0.01   | $W\cdot m^{-1}\cdot K^{-1}$ |
| $\rho$            | 150    | $kg\cdot m^{-3}$ |
| $c_p$             | 2000   | $J\cdot kg^{-1}\cdot K^{-1}$ |
| $\kappa$          | 40     | $m^{-1}$ |
| $q_k(x, 0, t)$    | -5     | $W\cdot m^{-2}$ |
| $T(x, -0.2 m, t)$ | 263.15 | $K$ |

As in the previous step, a complete input file will be explained block by block. It shall
be assumed that the input file is named `~/projects/problems/verification/2d_main.i`.
This file can be created and edited using any text editor program.

The problem requires a two-dimensional rectangular domain, which can be defined using
the [Mesh System](syntax/Mesh/index.md) as defined in the `[Mesh]` block  of the input.
The number of elements ("nx" and "ny") are defined to result in square elements.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=Mesh

There is a single unknown, temperature ($T$), to compute. This unknown is declared using the
[Variables System](syntax/Variables/index.md) in the `[Variables]` block and used the default
configuration of a first-order Lagrange finite element variable.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=Variables

The initial condition ($T_0$) is applied using the [Initial Condition System](syntax/ICs/index.md)
in the `[ICs]` block. In this case a constant value throughout the domain is provided.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=ICs

The volumetric heat source, $\dot{q}$, is defined using the
[Function System](syntax/Functions/index.md) using the `[Functions]` block. The function, as defined
in [tutorial03_heat_source] can be defined directly in the input file using the parsed function
capability. The "vars" and "vals" parameters are used to define named constants within the
equation. The variables "x", "y", "z", and "t" are always available and represent the
corresponding spatial location and time.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=Functions

The "volumetric" portion of the weak form are defined using the
[Kernel System](syntax/Kernels/index.md) in the `[Kernels]` block, for this example this
can be done with the use of three `Kernel` objects as follows.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=Kernels

The sub-block "T_time" defines the time derivative, "T_cond" defines the conduction portion
of the equation, and "T_source" defines the heat source term, please refer to
[tutorial03_verification/step01_heat_conduction.md] for details regarding the weak form of the heat
equation. All blocks include the "variable" parameter, which
is set equal to "T". The remaining parameters define the values for $k$, $\rho$, and $c_p$ as the
constant values defined in [tutorial03-snow-values].

The boundary portions of the weak form are defined using the
[Boundary Condition System](syntax/BCs/index.md) in the `[BCs]` block. At top of the domain ($y=0$) a
Neumann condition is applied with a constant outward flux. On the button of the domain
($y=-0.2 \textrm{m}$) a constant temperature is defined.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=BCs

The remain boundaries are "insulated", which is known as the natural boundary condition.
This does not require definition, as the name suggests, this condition is
applied naturally based on the weak form derivation.

The problem is solved using Newton's method with a first-order backward Euler method with a timestep
of 600 seconds (10 min.) up to a simulation time of nine hours. These settings are applied within the
`[Executioner]` block using the [Executioner System](syntax/Executioner/index.md).

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=Executioner

Finally, the output method is defined. In this case the ExodusII format is enabled within the
`[Outputs]` block using the [Outputs System](syntax/Outputs/index.md).

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=Outputs

### Simulation Execution

Executing the simulation is straightforward, simply execute the heat conduction module executable
with the input file included using the "-i" option as follows.

```
cd ~/projects/problems/verification
../../moose/modules/heat_conduction/heat_conduction-opt -i 2d_main.i
```

When complete an output file will be produced with the name "2d_main_out.e", this file
can be viewed using [Paraview](https://www.paraview.org/) or [python/peacock.md].
[tutorial03-snow-results] show the change in temperature across the domain with time. A key
feature is that the surface remains cool as the heat source warms the snow internally, creating large
temperature gradient at the surface.

!media tutorial03_verification/2d_main.mp4 id=tutorial03-snow-results
       caption=Simulation results for application of the heat equation for simulating snow in two-dimensions.


## MMS Overview

The [!ac](MMS) is a method of manufacturing a known solution for a [!ac](PDE), in this
case the transient heat equation. The method is explained in detail in the context of the
`mms` python package included with [!ac](MOOSE): [python/mms.md]. A brief overview shall be
provided here, but the reader is referred to the package documentation for a more detailed
description.

The method can be summarized into three basic steps:

1. Define an assumed solution for the [!ac](PDE) of interest, for example $T=2tx^2$.

   !alert tip title=Select a solution that cannot be represented exactly.
   The form of the solution is arbitrary, but certain characteristics are desirable. Mainly, a
   solution should be selected that cannot be exactly represented by the [!ac](FEM) shape function or
   the numerical integration scheme, for spatial and temporal convergence studies, respectively.
   Without these characteristic the assumed solution could possibly be solved exactly, thus there will not be
   error and the rate of convergence to the solution cannot be examined.

2. Apply the assumed solution to the [!ac](PDE) to compute a residual function. For example, consider
   the following simple [!ac](PDE).

   !equation
   \frac{\partial T}{\partial x} + t = 0

   Substitute the assumed solution of $t=2tx^2$ into this equation to compute the residual.

   !equation
   \frac{\partial}{\partial x}(2tx^2) + t = 4tx


3. Apply the negative of the computed function and perform the desired convergence study. For example,
   the example [!ac](PDE) above becomes:

   !equation
   \frac{\partial T}{\partial x} + t - 4tx = 0

   This [!ac](PDE) has a known solution, $T=2tx^2$, that can be used to compute the error and
   perform a convergence study.

While it is possible to perform these steps with hand calculations, this can become difficult and
error prone for more complex equations. For this reason the `mms` python package included with
[!ac](MOOSE) was created. This package will be used to apply these steps to the two-dimensional
heat equation example defined above.

## Spatial Convergence

The first step of using the [!ac](MMS) involves assuming a solution to the desired [!ac](PDE), which
in this case is the transient heat equation with a source term. [tutorial03_spatial_solution]
shall be the assumed solution for the spatial study.

!equation id=tutorial03_spatial_solution
T = t\sin(\pi x)\cdot\sin(5\pi y)

The assumed solution has two characteristics: it cannot be exactly represented by the
first-order [!ac](FEM) shape functions and the first-order time integration can exactly capture the time
portion of the equation and not result in temporal error accumulation.

The `mms` package, as shown in [tutorial03_step04_function], is used to compute the necessary
forcing function. The package can directly output the the input file format for the computed
forcing function and the assumed solution, making adding it to the input file trivial.

!listing tutorial03_verification/app/test/tests/step04_mms/step04_function.py id=tutorial03_step04_function link=false start=MooseDocs:start:spatial end=MooseDocs:end:spatial include-start=0
         caption=Compute the forcing function using [!ac](MMS).

Executing this script, assuming a name of `spatial_function.py` results in the following output.

!listing
$ python spatial_function.py
[mms_force]
  type = ParsedFunction
  value = 'cp*rho*sin(x*pi)*sin(5*y*pi) + 26*pi^2*k*t*sin(x*pi)*sin(5*y*pi) - shortwave*exp(y*kappa)*sin((1/2)*x*pi)*sin((1/3600)*pi*t/hours)'
  vars = 'hours rho shortwave k cp kappa'
  vals = '1.0 1.0 1.0 1.0 1.0 1.0'
[]
[mms_exact]
  type = ParsedFunction
  value = 't*sin(x*pi)*sin(5*y*pi)'
[]

Obviously, when adding to the input file the "vals" must be updated to the correct values for the
desired simulation.

[!ac](MOOSE) allows for multiple input files to be supplied to an executable, when this is done
the inputs are combined into a single file that comprises the union of the two files. As such,
the additional blocks required to perform the spatial convergence study and be created in
an independent file. It shall be assumed this file is named
`~/projects/problems/verification/2d_mms_spatial.i`.

The forcing function and exact solution are added using a parsed function, similar to the
existing functions within the simulation.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_mms_spatial.i link=false block=Functions

The forcing function is applied to the simulation by adding another heat source `Kernel` object.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_mms_spatial.i link=false block=Kernels

To perform the study the error and element size are required, these are added as `Postprocessors`
as done in the previous example in [Step 03](tutorial03_verification/step03_analytical_solution.md).

!listing tutorial03_verification/app/test/tests/step04_mms/2d_mms_spatial.i link=false block=Postprocessors

CSV output also needs to be added,

!listing tutorial03_verification/app/test/tests/step04_mms/2d_mms_spatial.i link=false block=Outputs

At this point, it is tempting to perform the study and the reader is encouraged to do so; however,
the results will be disappointing. The reason is that the initial and boundary conditions for the
simulation do not satisfy the assumed solution. In most cases, unless the study is testing the
boundary condition object specifically, the easiest approach is to use the assumed solution
to set both the initial condition and boundary conditions. This can be done by adding
function based initial and boundary condition objects, and setting the "active" line which
will disable all sub-blocks not listed.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_mms_spatial.i link=false block=BCs ICs

As was the case in the previous step of this tutorial, the `mms` package can be used to perform
the convergence study. In this case, both a first and second-order shape functions are considered.

!listing tutorial03_verification/app/test/tests/step04_mms/step04_study.py id=tutorial04-spatial-study
         caption=Example use of the `mms` package to run and plot spatial convergence.
         link=False start=MooseDocs:start:spatial end=MooseDocs:end:spatial include-start=False

The resulting convergence data indicate that the simulation is converging to the known solution at
the correct rate for first and second-order elements, thus verifying that the finite element solution
is computing the results as expected.

!media tutorial03_verification/2d_mms_spatial.png
       id=tutorial03-spatial-graph
       caption=Results of the spatial convergence study comparing the finite element simulation with
               the assumed two-dimensional solution.


## Temporal Convergence

The temporal convergence study is performed in the same manner as the spatial study. First,
an assumed solution is selected as defined in [tutorial03_temporal_solution]. In this case, the
spatial portion can be represented exactly by the shape functions and the temporal function
cannot be represented by the numerical time integration scheme. A decay function is selected that
has minimal changes in time and tends to an easy-to-recognize solution of `xy`.

!equation id=tutorial03_temporal_solution
T = x\cdot y\cdot\textrm{exp}(-1/32400 t)

Again, the `mms` package, as shown in [tutorial03_temporal_function], is used to compute the necessary
forcing function. The package can directly output the the input file format of the computed
forcing function and the assumed solution, making adding it to the input file trivial.

!listing tutorial03_verification/app/test/tests/step04_mms/step04_function.py id=tutorial03_temporal_function link=false start=MooseDocs:start:temporal end=MooseDocs:end:temporal include-start=0
         caption=Compute the forcing function using [!ac](MMS).

Executing this script, assuming a name of `temporal_function.py` results in the following output.

!listing
$ python temporal_function.py
[mms_force]
  type = ParsedFunction
  value = '-3.08641975308642e-5*x*y*cp*rho*exp(-3.08641975308642e-5*t) - shortwave*exp(y*kappa)*sin((1/2)*x*pi)*sin((1/3600)*pi*t/hours)'
  vars = 'kappa rho shortwave cp hours'
  vals = '1.0 1.0 1.0 1.0 1.0'
[]
[mms_exact]
  type = ParsedFunction
  value = 'x*y*exp(-3.08641975308642e-5*t)'
[]

A third input file, `~/projects/problems/verification/2d_mms_temporal.i` is created. The content
is nearly identical to the spatial counter part, as such is included completely below.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_mms_temporal.i link=false

The only differences are the function definitions and the use of time step size instead of the
element size computation.

Again, the `mms` package can be used to perform the study.

!listing tutorial03_verification/app/test/tests/step04_mms/step04_study.py id=tutorial04-temporal-study
         caption=Example use of the `mms` package to run and plot spatial convergence.
         link=False start=MooseDocs:start:temporal end=MooseDocs:end:temporal include-start=False

The resulting convergence data indicate that the simulation is converging to the known solution at
the correct rate for first and second-order time integration schemes, thus verifying that the
time integration is computing the correct results.

!media tutorial03_verification/2d_mms_temporal.png
       id=tutorial03-temporal-graph
       caption=Results of the temporal convergence study comparing the finite element simulation with
               the assumed two-dimensional solution.

## Closing Remarks

The above example demonstrates that a convergence study may be performed on simulations without
a known solution by manufacturing a solution. Performing a convergence study is crucial to building
robust simulation tools, as such a python package is provided to aid in performing the analysis.

!content pagination previous=tutorial03_verification/step03_analytical_solution.md
