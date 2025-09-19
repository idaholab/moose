# Step 2: Diffusion id=ictp_step2

!---

## Diffusion Problem

Let's solve a diffusion problem on our pin mesh to introduce the solving of a basic finite element problem.

Consider the steady-state diffusion equation on the domain $\Omega$ (our pin cell): find $u$ such that

!equation
-\nabla \cdot \nabla u = 0 \in \Omega,

where $u = 0$ on the interior and $u = 1$ on the exterior.

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

!listing moose/step2-1_diffusion.i block=Variables

Variables can be thought of as the equation(s) you are trying to solve.

The default parameters designate first-order linear Lagrange shape functions, thus without any additional parameters we end up with these shape functions.

!--

## Kernel Definition

Define a [`Diffusion`](Diffusion.md) kernel on the variable `u`:

!listing moose/step2-1_diffusion.i block=Kernels

A Kernel represents an object that contributes to the residual and the Jacobian matrix. Multiple can be used and they can also be coupled to one another to add nonlinearity.

Kernels can be "block restricted" (applied to a subset of the physical mesh) via the `block` parameter.

!--

## Boundary Condition Definition

Define two [`DirichletBC`](DirichletBC.md) boundary conditions on the variable `u`; one on the boundary `inner` with a value of `0` and one on the boundary `outer` with a value of `1`:

!listing moose/step2-1_diffusion.i block=BCs

!--

## Executioner Definition

Define a [`Steady`](Steady.md) executioner:

!listing moose/step2-1_diffusion.i block=Executioner

The other common executioner is the [`Transient`](Transient.md) executioner, which enables transient simulation.

!--

## Output Definition

Use the shorthand `Outputs/exodus` syntax for enabling Exodus output:

!listing moose/step2-1_diffusion.i block=Outputs

The shorthand syntax effectively creates a [`Exodus`](Exodus.md) output where the output files are created with the name `<input_file_name>_out.e`.

Other common shorthand syntax are `Outputs/csv` for CSV output and `Outputs/nemesis` for Nemesis (distributed Exodus) field output.

!--

## Input: Diffusion Problem

!listing moose/step2-1_diffusion.i

!---

## Run: Diffusion Problem

```bash
cardinal-opt -i step2-1_diffusion.i
```
!---

## Result: Diffusion Problem

!style halign=center
!media step2-1_solution.png style=width:50%

!--

## Diffusion with Volumetric Source

Let's replace the outer boundary condition (Dirichlet setting $u$ = 1) with a volumetric source represented by the function $10^9 * \sqrt{x^2 + y^2}$.

We will remove the [`DirichletBC`](DirichletBC.md) named `outer_dirichlet` and add a new Kernel of type [`BodyForce`](BodyForce.md).

!--

## Input: Diffusion with Volumetric Source

!listing moose/step2-2_diffusion_volumetric_source.i diff=moose/step2-1_diffusion.i

!---

## Run: Diffusion with Volumetric Source

```bash
cardinal-opt -i step2-2_diffusion_volumetric_source.i
```

!---

## Result: Diffusion with Volumetric Source

!style halign=center
!media step2-2_solution.png style=width:50%
