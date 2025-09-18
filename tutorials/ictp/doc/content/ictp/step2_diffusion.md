# Step 2: Diffusion id=ictp_step2

!---

## Basic Diffusion Problem

Let's solve a basic diffusion problem on our pin mesh.

Consider the steady-state diffusion equation on the domain $\Omega$ (our pin cell): find $u$ such that

!equation
-\nabla \cdot \nabla u = 0 \in \Omega,

where $u = 0$ and $u = 1$ on the exterior.

The weak form of this equation, in inner-product notation, is given by:

!equation
\left(\nabla \psi_i, \nabla u_h\right) = 0 \quad \forall \psi_i,

!--

## Required MOOSE Systems

We will utilize the following systems in MOOSE in addition to `[Mesh]` in our input:

- `[Variables]`: Define the unknown(s) of the problem

  - The variable $u$

- `[Kernels]`: Define the equation(s) to solve

  - The diffusion operator

- `[BCs]`: Define the boundary condition(s) of the problem

  - Dirichlet boundary conditions on the interior and exterior

- `[Executioner]`: Define how the problem will be solved

  - A steady state problem

- `[Outputs]`: Define how the solution will be returned

  - Exodus output for field visualization

We will go through each of these individually.

!--

## Variable Definition

Define the variable $u$:

!listing ictp/step2_diffusion.i block=Variables

The default parameters designate first-order linear Lagrange shape functions.

!--

## Kernel Definition

Define a [`Diffusion`](Diffusion.md) kernel on the variable `u`:

!listing ictp/step2_diffusion.i block=Kernels

!--

## Boundary Condition Definition

Define two [`DirichletBC`](DirichletBC.md) boundary conditions on the variable `u`; one on the boundary `inner` with a value of `0` and one on the boundary `outer` with a value of `1`:

!listing ictp/step2_diffusion.i block=BCs

!--

## Executioner Definition

Define a [`Steady`](Steady.md) executioner:

!listing ictp/step2_diffusion.i block=Executioner

The other common executioner is the [`Transient`](Transient.md) executioner, which enables transient simulation.

!--

## Output Definition

Use the shorthand `Outputs/exodus` syntax for enabling Exodus output:

!listing ictp/step2_diffusion.i block=Outputs

The shorthand syntax effectively creates a [`Exodus`](Exodus.md) output where the output files are created with the name `<input_file_name>_out.e`.

!--

## Input: Diffusion Problem

!listing ictp/step2_diffusion.i diff=ictp/step1-3_concentric_circle_no_void.i

!---

## Run: Diffusion Problem

```bash
cardinal-opt -i step2_diffusion.i
```

!---

## Result: Diffusion Problem

!style halign=center
!media step2-1_solution.png style=width:50%
