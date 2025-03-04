# Step 1: Geometry and Diffusion id=step01

!---

## Mesh Generation

Meshes can be generated in MOOSE using [mesh generators](Mesh/index.md).

2D slice of the shield domain using [GeneratedMeshGenerator](GeneratedMeshGenerator.md):

!row!

!col! width=50%

!style fontsize=60%
!listing step01_diffusion/mesh_part1.i

!col-end!

!col! width=50%

!media shield_multiphysics/results/mesh_part1.png style=width:80%;margin-left:auto;margin-right:auto;display:block

!col-end!

!row-end!

!---

Capturing features of the geometry using [CartesianMeshGenerator](CartesianMeshGenerator.md):

!row!

!col! width=50%

!style fontsize=60%
!listing step01_diffusion/mesh_part2.i
         diff=step01_diffusion/mesh_part1.i

!col-end!

!col! width=50%

!media shield_multiphysics/results/mesh_part2.png style=width:80%;margin-left:auto;margin-right:auto;display:block

!col-end!

!row-end!

!---

Specifying subdomains or "blocks" to capture differing materials:

!row!

!col! width=50%

!style fontsize=60%
!listing step01_diffusion/mesh_part3.i
         diff=step01_diffusion/mesh_part2.i

!col-end!

!col! width=50%

!media shield_multiphysics/results/mesh_part3.png style=width:80%;margin-left:auto;margin-right:auto;display:block

!col-end!

!row-end!

!---

Extending to 3D with "slices" of the domain:

!row!

!col! width=50%

!style fontsize=60%
!listing step01_diffusion/mesh_part4.i
         diff=step01_diffusion/mesh_part3.i

!col-end!

!col! width=50%

!media shield_multiphysics/results/mesh_part4.png
       caption=Z-axis not to scale
       style=width:90%;margin-left:auto;margin-right:auto;display:block

!col-end!

!row-end!

!---

Finally, we can string together mesh generators to:

1. Remove blocks with [BlockDeletionGenerator](BlockDeletionGenerator.md)
2. Rename blocks to something more memorable with [RenameBlockGenerator](RenameBlockGenerator.md)

!row!

!col! width=50%

!style fontsize=60%
!listing step01_diffusion/mesh.i
         diff=step01_diffusion/mesh_part4.i

!col-end!

!col! width=50%

!media shield_multiphysics/results/mesh_part5.png
       style=width:90%;margin-left:auto;margin-right:auto;display:block

!col-end!

!row-end!

!---

## Diffusion Problem Statement

With this mesh, we first consider the steady-state diffusion equation on the domain $\Omega$: find $T$ such that

!equation
-\nabla \cdot \nabla T = 0 \in \Omega,

where $T = 300$ on the back ($z=0$), $T = 330$ on the front ($z=5.25$) and with
$\nabla T \cdot \hat{n} = 0$ on the remaining boundaries.

The weak form of this equation, in inner-product notation, is given by:

!equation
\left(\nabla \psi_i, \nabla T_h\right) = 0 \quad \forall \psi_i,

where $\psi_i$ are the test functions and $T_h$ is the finite element solution.

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

## Step 1: Input file to run the diffusion simulation

!listing step01_diffusion/step1.i

!---

## Step 1: Run

An executable is available from loading the MOOSE conda or HPC module.

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/inputs/step01_diffusion
moose-opt -i mesh.i --mesh-only
moose-opt -i step1.i
```

!---

## Step 1: Result

!media shield_multiphysics/results/step01.png
