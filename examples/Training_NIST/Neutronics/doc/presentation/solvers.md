# Solvers

!---

# DFEM-SN Solver Strategy

!row!
!col! width=80%
- Extremely large coupled system of equations

  - Matrix is too large to store
  - Non-symmetric system difficult to solve
  - Need to utilize physics-based iterative solvers

- Iteration scheme with acceleration:

  - Streaming + Collision inversion via sweeping
  - Scattering + Fission updated as source
  - Coarse-mesh finite difference (CMFD) acceleration

!media cmfd.png
!col-end!

!col! width=20%

!media cm_overlay.png

!media grid_overlay.png

!col-end!
!row-end!

!---

# Steady-State DFEM-SN

!row!
!col! width=69%
!style! fontsize=70%

- Two meshes are created/loaded:

  - Fine mesh includes all the geometric details and material regions
  - Coarse mesh has the same domain, but contains coarser elements
  - Coarse mesh is overlayed on fine mesh to assing IDs

- Iteration parameters:

  - `richardson_max_its`: maximum number of sweeper-CMFD iterations
  - `richardson_max/rel_tol`: defines convergence criteria
  - `richardson_value`: displays value of postprocessor each iteration

- Sweeper parameters:

  - `inner_solve_type/max_inner_its`: performs multiple sweeps per iteration to improve stability

- CMFD parameters (`cmfd_accleration = true`):

  - `coarse_element_id`: ID name defining the coarse mesh
  - `group_collapsing`: Defines the projection of energy groups
  - `prolongation_type`: additive or multiplicative prolongation

!style-end!
!col-end!

!col! width=1%
!!
!col-end!

!col! width=30%
!style! fontsize=80%
```
[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = fine_mesh.e
  []
  [coarse_mesh]
    type = FileMeshGenerator
    file = coarse_mesh.e
  []
  [assign_coarse_element_id]
    type = CoarseMeshExtraElementIDGenerator
    input = fmg
    coarse_mesh = coarse_mesh
    extra_element_id_name = coarse_element_id
  []
[]

[Executioner]
  type = SweepUpdate

  richardson_max_its = 1000
  richardson_abs_tol = 1e-8
  richardson_rel_tol = 1e-6
  richardson_value = eigenvalue

  inner_solve_type = GMRes
  max_inner_its = 2

  cmfd_acceleration = true
  coarse_element_id = coarse_element_id
  group_collapsing = '0 1'
  prolongation_type = multiplicative
[]
```
!style-end!
!col-end!
!row-end!

!---

# Transient DFEM-SN

```
[Executioner]
  type = TransientSweepUpdate

  end_time = 100
  [TimeStepper]
    type = ConstantDT
    dt = 1
  []

  richardson_max_its = 1000
  richardson_abs_tol = 1e-8
  richardson_rel_tol = 1e-6

  inner_solve_type = GMRes
  max_inner_its = 2

  cmfd_acceleration = true
  coarse_element_id = coarse_element_id
  group_collapsing = '0 1'
  prolongation_type = multiplicative
[]
```

!---

# Steady-state CFEM-Diffusion

!row!
!col! width=69%
- Using vanilla `Eigenvalue` executioner from MOOSE
- `PJFNKMO` solve type with `constant_matrices` is the fastest solve method
- Using algebraic multi-grid (AMG) from HYPRE library is usually the best preconditioner
- Setting a large GMRes restart helps prevent divergence
!col-end!

!col! width=1%
!!
!col-end!

!col! width=30%
```
[Executioner]
  type = Eigenvalue
  solve_type = PJFNKMO
  constant_matrices = true

  petsc_options_iname = '-pc_type
                         -pc_hypre_type
                         -ksp_gmres_restart'
  petsc_options_value = 'hypre
                         boomeramg
                         100'
[]
```
!col-end!
!row-end!

!---

# Transient CFEM-Diffusion

```
[Executioner]
  type = Transient
  solve_type = NEWTON

  end_time = 100
  [TimeStepper]
    type = ConstantDT
    dt = 1
  []

  petsc_options_iname = '-pc_type -pc_hypre_type -ksp_gmres_restart'
  petsc_options_value = 'hypre boomeramg 100'
[]
```

!---

# Initial Condition for Transient Simulation

- Transient neutronics typically start from a steady-state condition
- Must solve a eigenvalue problem and set the transient initial condition to this solution
- Two strategies:

  - Using MultiApps and Transfers to solve the initial simulation
  - Solve the initial simulation, save to file, and load file when performing transient

!--

# Initial Condition from MultiApp

```
[MultiApps]
  [initial]
    type = FullSolveMultiApp
    input_files = initial.i
    execute_on = INITIAL
  []
[]

[Transfers]
  [initial_transfer]
    type = TransportSystemVariableTransfer
    from_multi_app = initial
    from_transport_system = diff
    to_transport_system = diff
  []
[]
```

!---

# Initial Condition from File

!row!
!col! width=45%
### Option 1: Restart from exodus (only for CFEM)

```
[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = initial_out.e
    use_for_exodus_restart = true
  []
[]

[TransportSystems]
  ...
  restart_transport_system = true
[]
```
!col-end!

!col! width=10%
!!
!col-end!

!col! width=45%
### Option 2: Load from binary

Initial input:

```
[UserObjects]
  [initial]
    type = TransportSolutionVectorFile
    transport_system = diff
    writing = true
    execute_on = 'final'
  []
[]
```

Transient input:

```
[UserObjects]
  [initial]
    type = TransportSolutionVectorFile
    transport_system = diff
    writing = false
    execute_on = 'initial'
  []
[]
```

!col-end!
!row-end!
