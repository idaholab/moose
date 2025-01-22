# Step 1: Geometry and Diffusion id=step01

!---

First, we consider the steady-state diffusion equation on the domain $\Omega$: find $u$ such that

!equation
-\nabla \cdot \nabla u = 0 \in \Omega,

where $u = 300$ on the left, $u = 200$ on the right and with
$\nabla u \cdot \hat{n} = 0$ on the remaining boundaries.

The weak form of this equation, in inner-product notation, is given by:

!equation
\left(\nabla \psi_i, \nabla u_h\right) = 0 \quad \forall \psi_i,

where $\psi_i$ are the test functions and $u_h$ is the finite element solution.

!---

## Input File(s)

An input file is used to represent the problem in MOOSE. It follows a very standardized
syntax.

MOOSE uses the "hierarchical input text" (hit) format.

!listing step01_diffusion/step1.i block=Kernels

!---

A basic MOOSE input file requires six parts, each of which will be covered in greater detail later.

- `[Mesh]`: Define the geometry of the domain
- `[Variables]`: Define the unknown(s) of the problem
- `[Kernels]`: Define the equation(s) to solve
- `[BCs]`: Define the boundary condition(s) of the problem
- `[Executioner]`: Define how the problem will be solved
- `[Outputs]`: Define how the solution will be returned

!---

## Step 1: Input file to build the geometry

!listing step01_diffusion/mesh.i

!---

## Step 1: Input file to run the diffusion simulation

!listing step01_diffusion/step1.i

!---

## Step 1: Run

An executable is produced by compiling an application or a MOOSE module. It can be used
to run input files.

If using the local tutorial executable

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/step01_diffusion
../executable/shield_multiphysics-opt -i mesh.i --mesh-only
../executable/shield_multiphysics-opt -i step1.i
```

If using the conda MOOSE executable:

```bash
conda activate moose
cd ~/projects/moose/tutorials/shield_multiphysics/step01_diffusion
moose-opt -i mesh.i --mesh-only
moose-opt -i step1.i
```

!---

## Step 1: Result

!media shield_multiphysics/results/step01.png style=width:70%;margin-left:auto;margin-right:auto
