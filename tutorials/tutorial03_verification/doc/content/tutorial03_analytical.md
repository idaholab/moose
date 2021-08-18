# Analytic Solution

!---

## Summary

The best method for verifying a simulation is comparison with a known mathematical solution.
For the heat equation, one such solution shall be considered to verify a model. The verification
process shall be broken into four parts:

1. Define a problem, with a known exact solution.
2. Simulate the problem.
3. Compute the error between the known solution and simulation.
4. Perform convergence study.

!---

## Theory: Problem Statement

[!cite](incropera1996fundamentals) presents a one-dimensional solution for the heat equation as:

!equation
T(t,x) = T_0 + \frac{\dot{q}}{k} \Bigg\lbrack 2 \sqrt{\frac{\alpha t}{\pi}}
         \exp\bigg\lparen{\frac{-x^2}{4 \alpha t}}\bigg\rparen -
         x \bigg\lparen 1-\textrm{erf}\bigg\lparen\frac{x}{2\sqrt{\alpha t}} \bigg\rparen \bigg\rparen \Bigg\rbrack,

where $T$ is the temperature, $T_0$ is the initial temperature, $t$ is time, $x$ is position,
$\dot{q}$ is the heat source, $k$ is thermal conductivity, and $\alpha$ is the thermal diffusivity.
Thermal diffusivity is defined as the ratio of thermal conductivity ($k$) to the product of density ($\rho$)
and specific heat capacity ($c_p$). The system is subjected to Neumann boundary conditions at the
left boundary ($x=0$) and the natural boundary condition on the opposite boundary.

!---

!media tutorial03_verification/1d_exact.mp4 style=width:75%;margin-left:auto;margin-right:auto;display:block;

!---

## Practice: Simulation

[!ac](MOOSE) operates using input files, as such, an input file shall be detailed that is capable of
simulating this problem and its use demonstrated.

The location of this input file is arbitrary and it can be created and edited using any text editor
program.

!---

### Domain

The domain of the problem is defined using the Mesh System using the `[Mesh]` block of the input file.

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Mesh

!---

### Variable

There is a single unknown, temperature ($T$), to compute. This unknown is declared using the
Variables System in the `[Variables]` block and used the default
configuration of a first-order Lagrange finite element variable.

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Variables

!---

### Kernels

The "volumetric" portions equation weak form are defined using the Kernel System in the `[Kernels]`
block.

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Kernels

!---

### Boundary Conditions

The boundary portions of the equation weak form are defined using the
Boundary Condition System in the `[BCs]` block. At $x=0$ a
Neumann condition is applied with a value of 7E5, by default $x=0$ is given the name of "left".

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=BCs

!---

### Execution

The problem is solved using Newton's method with a second-order backward difference formula (BDF2)
with a timestep of 0.01 seconds up to a simulation time of one second. These settings are applied
within the `[Executioner]` block using the Executioner System.

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Executioner

!---

### Output

The ExodusII and CSV format is enabled within the `[Outputs]` block using the Outputs
System.

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Outputs

!---

## Practice: Run

Executing the simulation is straightforward, simply execute the heat conduction module executable
with the input file included using the "-i" option as follows.

```
~/projects/moose/modules/heat_conduction/heat_conduction-opt -i 1d_analytical.i
```

!---

## Practice: Compute Error

To compute the error the exact solution must be added, from which the error can be computed using
the [!ac](FEM) solution.

!---


### Exact Solution

The known solution can be defined using the Function System in the `Functions` block.

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Functions

The "vars" and "vals" have a one-to-one relationship, e.g., $k=80.2$. If defined in this manner
then the names in "vars" can be used in the definition of the equation in the "value" parameter.

!---

### $L_2$ Error

The $L_2$-norm of the differnce between the computed and exact solution
can be computed using the `NodalL2Error` object. This is created within the `[Postprocessors]` block
along with the average element size.

!listing tutorial03_verification/app/test/tests/step03_analytical/1d_analytical.i link=False block=Postprocessors

!---

!media tutorial03_verification/1d_analytical.mp4 style=width:75%;margin-left:auto;margin-right:auto;display:block;

!---

## Practice: Convergence Rate

[!ac](MOOSE) includes a python package---the `mms` package---for performing convergence studies, as
such shall be used here.

!---

### Spatial Convergence

To perform a spatial convergence the input file create needs to be executed with decreasing element
size, which in [!ac](MOOSE) can be done by adding the `Mesh/uniform_refine=x` option to the
command line with "x" being an integer representing the number of refinements to perform.

!listing tutorial03_verification/app/test/tests/step03_analytical/step03_study.py
         link=False start=MooseDocs:start:spatial end=MooseDocs:end:spatial include-start=False

!---

!media tutorial03_verification/1d_analytical_spatial.png

!---

### Temporal

The temporal convergence study is nearly identical to the spatial.
For a temporal study, the time step is reduced.

!listing tutorial03_verification/app/test/tests/step03_analytical/step03_study.py
         link=False start=MooseDocs:start:temporal end=MooseDocs:end:temporal include-start=False

!---

!media tutorial03_verification/1d_analytical_temporal.png

!---

## Practice: Closing Remark

The resulting convergence data indicate that the simulation is converging to the known solution
at the correct rate for linear shape functions and second-order time integration. Thus, it can
be stated that this simulation is +verified+.
