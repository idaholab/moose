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

An executable is available from loading the MOOSE conda or HPC module.

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/inputs/step01_diffusion
moose-opt -i mesh_part1.i --mesh-only
```

Console output in `mesh-only` mode shows a summary of the mesh.

```
 Mesh Information:
  elem_dimensions()={2}
  elem_default_orders()={FIRST}
  supported_nodal_order()=1
  spatial_dimension()=2
  n_nodes()=88
    n_local_nodes()=88
  n_elem()=70
    n_local_elem()=70
    n_active_elem()=70
  n_subdomains()=1
  n_elemsets()=0
  n_partitions()=1
  n_processors()=1
  n_threads()=1
  processor_id()=0
  is_prepared()=true
  is_replicated()=true

 Mesh Bounding Box:
  Minimum: (x,y,z)=(       0,        0,        0)
  Maximum: (x,y,z)=(     6.5,      9.7,        0)
  Delta:   (x,y,z)=(     6.5,      9.7,        0)

 Mesh Element Type(s):
  QUAD4

 Mesh Nodesets:
  Nodeset 0 (bottom), 8 nodes
   Bounding box minimum: (x,y,z)=(       0,        0,        0)
   Bounding box maximum: (x,y,z)=(     6.5,        0,        0)
   Bounding box delta: (x,y,z)=(     6.5,        0,        0)
  Nodeset 1 (right), 11 nodes
   Bounding box minimum: (x,y,z)=(     6.5,        0,        0)
   Bounding box maximum: (x,y,z)=(     6.5,      9.7,        0)
   Bounding box delta: (x,y,z)=(       0,      9.7,        0)
  Nodeset 2 (top), 8 nodes
   Bounding box minimum: (x,y,z)=(       0,      9.7,        0)
   Bounding box maximum: (x,y,z)=(     6.5,      9.7,        0)
   Bounding box delta: (x,y,z)=(     6.5,        0,        0)
  Nodeset 3 (left), 11 nodes
   Bounding box minimum: (x,y,z)=(       0,        0,        0)
   Bounding box maximum: (x,y,z)=(       0,      9.7,        0)
   Bounding box delta: (x,y,z)=(       0,      9.7,        0)

 Mesh Sidesets:
  Sideset 0 (bottom), 7 sides (EDGE2), 7 elems (QUAD4), 8 nodes
   Side volume: 6.5
   Bounding box minimum: (x,y,z)=(       0,        0,        0)
   Bounding box maximum: (x,y,z)=(     6.5,        0,        0)
   Bounding box delta: (x,y,z)=(     6.5,        0,        0)
  Sideset 1 (right), 10 sides (EDGE2), 10 elems (QUAD4), 11 nodes
   Side volume: 9.7
   Bounding box minimum: (x,y,z)=(     6.5,        0,        0)
   Bounding box maximum: (x,y,z)=(     6.5,      9.7,        0)
   Bounding box delta: (x,y,z)=(       0,      9.7,        0)
  Sideset 2 (top), 7 sides (EDGE2), 7 elems (QUAD4), 8 nodes
   Side volume: 6.5
   Bounding box minimum: (x,y,z)=(       0,      9.7,        0)
   Bounding box maximum: (x,y,z)=(     6.5,      9.7,        0)
   Bounding box delta: (x,y,z)=(     6.5,        0,        0)
  Sideset 3 (left), 10 sides (EDGE2), 10 elems (QUAD4), 11 nodes
   Side volume: 9.7
   Bounding box minimum: (x,y,z)=(       0,        0,        0)
   Bounding box maximum: (x,y,z)=(       0,      9.7,        0)
   Bounding box delta: (x,y,z)=(       0,      9.7,        0)

 Mesh Edgesets:
  None

 Mesh Subdomains:
  Subdomain 0: 70 elems (QUAD4, 70 active), 88 active nodes
   Volume: 63.05
   Bounding box minimum: (x,y,z)=(       0,        0,        0)
   Bounding box maximum: (x,y,z)=(     6.5,      9.7,        0)
   Bounding box delta: (x,y,z)=(     6.5,      9.7,        0)
  Global mesh volume = 63.05
```

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

We generate the full mesh now with the consolidated meshing script:

```bash
cd ~/projects/moose/tutorials/shield_multiphysics/inputs/step01_diffusion
moose-opt -i mesh.i --mesh-only
```

!---

## Diffusion Problem Statement

With this mesh, we first consider the steady-state diffusion equation on the domain $\Omega$: find $T$ such that

!equation
-\nabla \cdot \nabla T = 0 \in \Omega,

where $T = 300$ on the back ($z=0$), $T = 330$ on the front ($z=5.25$) and with
$\nabla T \cdot \hat{n} = 0$ on the remaining boundaries.

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

The simulation header gives a lot of helpful information

```
Framework Information:
MOOSE Version:           git commit 77c5910099 on 2025-05-28
LibMesh Version:
PETSc Version:           3.23.0
SLEPc Version:           3.23.0
Current Time:            Wed Jun  4 22:26:52 2025
Executable Timestamp:    Mon Jun  2 08:29:52 2025

Input File(s):
  /Users/giudgl/projects/moose_v9/tutorials/shield_multiphysics/inputs/step01_diffusion/step1.i

Checkpoint:
  Wall Time Interval:      Every 3600 s
  User Checkpoint:         Disabled
  # Checkpoints Kept:      2
  Execute On:              TIMESTEP_END

Parallelism:
  Num Processors:          1
  Num Threads:             1

Mesh:
  Parallel Type:           replicated
  Mesh Dimension:          3
  Spatial Dimension:       3
  Nodes:                   21692
  Elems:                   17920
  Num Subdomains:          4

Nonlinear System:
  Num DOFs:                21692
  Num Local DOFs:          21692
  Variables:               "T"
  Finite Element Types:    "LAGRANGE"
  Approximation Orders:    "FIRST"

Execution Information:
  Executioner:             Steady
  Solver Mode:             NEWTON
  PETSc Preconditioner:    hypre boomeramg strong_threshold: 0.7 (auto)
  MOOSE Preconditioner:    SMP (auto)
 0 Nonlinear |R| = 3.409028e+03
      0 Linear |R| = 3.409028e+03
      1 Linear |R| = 1.290229e+02
      2 Linear |R| = 1.113660e+01
      3 Linear |R| = 8.340899e-01
      4 Linear |R| = 5.736450e-02
      5 Linear |R| = 2.859743e-03
 1 Nonlinear |R| = 2.859743e-03
      0 Linear |R| = 2.859743e-03
      1 Linear |R| = 2.670323e-04
      2 Linear |R| = 1.878905e-05
      3 Linear |R| = 9.979916e-07
      4 Linear |R| = 7.651689e-08
      5 Linear |R| = 4.971569e-09
 2 Nonlinear |R| = 4.971578e-09
 Solve Converged!
```

!---

## Step 1: Result

!media shield_multiphysics/results/step01.png
