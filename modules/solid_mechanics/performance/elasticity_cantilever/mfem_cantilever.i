# Linear elasticity cantilever, MFEM backend, for comparison against kokkos_cantilever.i.
#
# The same beam, boundary conditions, material and solver configuration as the libMesh input, so
# that the two measure the same problem. 'refinement' is the number of uniform refinements applied
# to beam-hex.mesh, chosen so that both backends reach the same number of degrees of freedom. The
# reference measurements use refinement = 5 (262144 elements, 839619 DoFs) on 24 MPI ranks.
#
# Refinement here is serial, applied before the mesh is partitioned into an mfem::ParMesh. The
# 'parallel_refine' parameter refines after partitioning instead, which is cheaper but changes what
# is being compared, since the libMesh backend has no equivalent for a generated mesh.

refinement = 3

[Mesh]
  type = MFEMFileMesh
  file = beam-hex.mesh
  uniform_refine = ${refinement}
  displacement = "displacement"
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMVectorFESpace
    fec_type = H1
    fec_order = FIRST
    range_dim = 3
    ordering = "vdim"
  []
[]

[Variables]
  [displacement]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [dirichlet]
    type = MFEMVectorDirichletBC
    variable = displacement
    boundary = '1'
  []
  [pull_down]
    type = MFEMVectorBoundaryIntegratedBC
    variable = displacement
    boundary = '2'
    vector_coefficient = '0.0 0.0 -0.01'
  []
[]

[FunctorMaterials]
  [Rigidium]
    type = MFEMGenericFunctorMaterial
    prop_names = 'lambda mu'
    prop_values = '60.5e9 25.9e9'
    block = 1
  []
  [Bendium]
    type = MFEMGenericFunctorMaterial
    prop_names = 'lambda mu'
    prop_values = '60.5e9 25.9e9'
    block = 2
  []
[]

[Kernels]
  [diff]
    type = MFEMLinearElasticityKernel
    variable = displacement
    lambda = lambda
    mu = mu
  []
[]

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
    fespace = H1FESpace
    vector_treatment = by_component
    l_max_its = 1
    l_tol = 0
    strength_threshold = 0.7
    aggressive_coarsening_levels = 1
    print_level = 1
  []
  [main]
    type = MFEMHyprePCG
    preconditioner = boomeramg
    l_max_its = 500
    l_tol = 1e-8
    print_level = 1
  []
[]

[Executioner]
  type = MFEMSteady
  device = "cpu"
[]

[Outputs]
  print_linear_residuals = false
  [perf]
    type = PerfGraphOutput
    level = 5
    heaviest_sections = 12
  []
[]
