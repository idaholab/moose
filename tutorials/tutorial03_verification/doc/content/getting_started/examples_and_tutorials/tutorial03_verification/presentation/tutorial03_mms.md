# Method of Manufactured Solution

!---

## Summary

In practice an exact solution is not known, since there would be no reason to perform a
simulation if the answer is known.

It is possible to "manufacture" a solution using the [!ac](MMS).

The two-dimensional heat equation will be used to simulate a physical system and used as basis
for spatial and temporal convergence study using the [!ac](MMS).

!---

## Theory: Method of Manufactured Solution

The [!ac](MMS) can be summarized into three basic steps:

1. Assume a solution for the [!ac](PDE) of interest: $T=2tx^2$

2. Apply assumed solution to compute a residual function. For example,

   !equation
   \begin{align*}
   \frac{\partial T}{\partial x} + t &= 0
   \quad \xrightarrow{\clap{assume}} \quad
   \frac{\partial}{\partial x}(2tx^2) + t = 4tx
   \end{align*}

3. Add the negative of the computed function and perform the desired convergence study. For example,
   the [!ac](PDE) above becomes:

   !equation
   \frac{\partial T}{\partial x} + t - 4tx = 0

   This [!ac](PDE) has a known solution, $T=2tx^2$, that can be used to compute the error and
   perform a convergence study.

!---

### Assumed Solution

The form of the solution is arbitrary, but certain characteristics are desirable to eliminate an unnecessary
accumulation of error.

- For a spatial study, the solution +should not+ be exactly represented by the shape
  functions and +should+ be exactly represented by the numerical integration scheme.
- For a temporal study, the solution +should+ be exactly represent by the shape functions and
  +should not+ be exactly represented by the numerical integration scheme.

!---

## Theory: Problem Statement

The two-dimensional heat equation shall be used to simulate a cross section
of snow subjected to radiative cooling at the surface and internal heating due to solar absorption.

- The domain is defined as $x \in [0, 1 \textrm{m}]$ and $y \in [-0.2 \textrm{m}, 0]$.
- A heat source, $\dot{q}$, is applied as an approximation of internal heating due to
  solar absorption. The source is raised from zero to a maximum of 650 $W\cdot m^{-3}$ back to zero
  during a 9 hours (32400 sec.) period following a sine function.  The duration is an approximation of the
  daylight hours in Idaho Falls, Idaho in December.
- The left side of the domain is shaded and the
  right is subjected to the maximum source, again defined using a sine function.
- The top surface ($y=0$) is subjected to a Neumann boundary condition, $k\nabla T
  \cdot \hat{n} = q_k(x,t)$, which approximates the radiative cooling of the snow surface.

!---

## Theory: Material Properties

| Variable          | Value  | Units |
| :-                | :-     | :-    |
| $T_0$             | 263.15 | $K$     |
| $k$               | 0.01   | $W\cdot m^{-1}\cdot K^{-1}$ |
| $\rho$            | 150    | $kg\cdot m^{-3}$ |
| $c_p$             | 2000   | $J\cdot kg^{-1}\cdot K^{-1}$ |
| $\kappa$          | 40     | $m^{-1}$ |
| $q_k(x, 0, t)$    | -5     | $W\cdot m^{-2}$ |
| $T(x, -0.2 m, t)$ | 263.15 | $K$ |

!---

## Practice: Simulation

The heat conduction module is capable of performing this simulation, thus only an input file is
needed to simulate this problem.

!---

### Domain

The problem requires a two-dimensional rectangular domain, which can be defined using
the Mesh System as defined in the `[Mesh]` block  of the input.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=Mesh

!---

### Variable

There is a single unknown, temperature ($T$), to compute. This unknown is declared using the
Variables System in the `[Variables]` block and used the default
configuration of a first-order Lagrange finite element variable.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=Variables

!---

### Initial Conduction

The initial condition ($T_0$) is applied using the Initial Condition System
in the `[ICs]` block. In this case a constant value throughout the domain is provided.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=ICs

!---

### Heat Source

The volumetric heat source, $\dot{q}$, is defined using the
Function System using the `[Functions]` block. The variables "x", "y", "z", and "t" are always
available and represent the corresponding spatial location and time.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=Functions

!---

### Kernels

The "volumetric" portions of the weak form of the equation are defined using the
Kernel System in the `[Kernels]` block, for this example this
can be done with the use of three `Kernel` objects as follows.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=Kernels

!---

### Boundary Conditions

The boundary portions of the equation weak form are defined using the
Boundary Condition System in the `[BCs]` block. At top of the domain ($y=0$) a
Neumann condition is applied with a constant outward flux. On the button of the domain
($y=-0.2 \textrm{m}$) a constant temperature is defined. The remain boundaries are "insulated",
which is known as the natural boundary condition.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=BCs

!---

### Execution

The problem is solved using Newton's method with a first-order backward Euler method with a timestep
of 600 seconds (10 min.) up to a simulation time of nine hours. These settings are applied within the
`[Executioner]` block using the Executioner System.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=Executioner

!---

### Output

The ExodusII and CSV format is enabled within the `[Outputs]` block using the Outputs
System.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_main.i link=False block=Outputs

!---

## Practice: Run

Executing the simulation is straightforward, simply execute the heat conduction module executable
with the input file included using the "-i" option as follows.

```
~/projects/moose/modules/heat_conduction/heat_conduction-opt -i 2d_main.i
```

!---

!media tutorial03_verification/2d_main.mp4 style=width:100%;margin-left:auto;margin-right:auto;display:block;

!---

## Practice: Convergence Rate

[!ac](MOOSE) includes a python package---the `mms` package---for performing convergence studies, as
such shall be used here.

!---

### Spatial Convergence: Assumed Solution

Assume a solution to the desired [!ac](PDE).

!equation
T = t\sin(\pi x)\cdot\sin(5\pi y)

- The solution cannot be exactly represented by the first-order shape functions.
- The first-order time integration can capture the time portion of the equation exactly, thus
  does not result in temporal error accumulation.

!---

### Spatial Convergence: Forcing Function


The `mms` package can compute the necessary forcing function and output the the input file syntax
for both the forcing function and the assumed solution.

!listing tutorial03_verification/app/test/tests/step04_mms/step04_function.py link=false start=MooseDocs:start:spatial end=MooseDocs:end:spatial include-start=0

!---

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

!---

### Spatial Convergence: Input

[!ac](MOOSE) allows for multiple input files to be supplied to an executable, when this is done
the inputs are combined into a single file that comprises the union of the two files.

The additional blocks required to perform the spatial convergence study and be created in
an independent file.

!---

#### Forcing and Solution Functions

The forcing function and exact solution are added using a parsed function, in similar to the
existing functions withing the simulation.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_mms_spatial.i link=false block=Functions

!---

#### Forcing Function as Heat Source

The forcing function is applied the the simulation by adding another heat source `Kernel` object.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_mms_spatial.i link=false block=Kernels

!---

#### Error and Element Size

To perform the study the error and element size are required, these are added as `Postprocessors`
block.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_mms_spatial.i link=false block=Postprocessors

!---

#### Output

CSV output also needs to be added, which is added as expected.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_mms_spatial.i link=false block=Outputs

!---

#### Boundary and Initial Conditions

The initial and boundary conditions for the simulation do not satisfy the assumed solution. The
easiest approach to remedy this is to use the assumed solution
to set both the initial condition and boundary conditions.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_mms_spatial.i link=false block=BCs ICs

!---

### Spatial Convergence: Run

The `mms` package can be used to perform the convergence study. In this case, both a first and
second-order shape functions are considered.

!listing tutorial03_verification/app/test/tests/step04_mms/step04_study.py
         link=False start=MooseDocs:start:spatial end=MooseDocs:end:spatial include-start=False

!---

!media tutorial03_verification/2d_mms_spatial.png

!---

### Temporal Convergence: Assumed Solution

Assume a solution to the desired [!ac](PDE).

!equation
T = x\cdot y\cdot\textrm{exp}(-1/32400 t)

- The solution can be exactly represented by the first-order shape functions, thus
  does not result in spatial error accumulation.
- The first-order time integration cannot capture the time portion of the equation exactly.


!---

### Temporal Convergence: Forcing Function

The `mms` package can compute the necessary forcing function and output the the input file syntax
for both the forcing function and the assumed solution.

!listing tutorial03_verification/app/test/tests/step04_mms/step04_function.py link=false start=MooseDocs:start:temporal end=MooseDocs:end:temporal include-start=0

!---

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

!---

### Temporal Convergence: Input

A third input is created. The content follows the input to the spatial counter part and as such is
included completely below.

!listing tutorial03_verification/app/test/tests/step04_mms/2d_mms_temporal.i link=false

!---

### Temporal Convergence: Run

The `mms` package can be used to perform the convergence study. In this case, both first and
second-order shape functions are considered.

!listing tutorial03_verification/app/test/tests/step04_mms/step04_study.py
         link=False start=MooseDocs:start:temporal end=MooseDocs:end:temporal include-start=False

!---

!media tutorial03_verification/2d_mms_temporal.png
