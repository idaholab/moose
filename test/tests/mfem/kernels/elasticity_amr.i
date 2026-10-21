# Linear elasticity for a two-material cantilever beam, clamped on boundary 1 and pulled down on
# boundary 2, with h-refinement driven by the L2 Zienkiewicz-Zhu indicator of the stress.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/beam-tet.mesh
[]

[Problem]
  type = MFEMProblem
[]

[FESpaces]
  [H1FESpace]
    type = MFEMVectorFESpace
    fec_type = H1
    fec_order = FIRST
    ordering = NODES
  []
[]

[Variables]
  [displacement]
    type = MFEMVariable
    fespace = H1FESpace
  []
[]

[BCs]
  [clamped]
    type = MFEMVectorDirichletBC
    variable = displacement
    boundary = 1
  []
  [pull_down]
    type = MFEMVectorBoundaryIntegratedBC
    variable = displacement
    boundary = 2
    vector_coefficient = '0.0 0.0 -0.01'
  []
[]

[FunctorMaterials]
  [Rigidium]
    type = MFEMGenericFunctorMaterial
    prop_names = 'lambda mu'
    prop_values = '50.0 50.0'
    block = 1
  []
  [Bendium]
    type = MFEMGenericFunctorMaterial
    prop_names = 'lambda mu'
    prop_values = '1.0 1.0'
    block = 2
  []
[]

[Kernels]
  [elasticity]
    type = MFEMLinearElasticityKernel
    variable = displacement
    lambda = lambda
    mu = mu
  []
[]

[Adaptivity]
  [Indicators]
    [l2zz]
      type = MFEML2ZienkiewiczZhuIndicator
      variable = displacement
      kernel = elasticity
    []
  []
  [Markers]
    [ref]
      type = MFEMRefinementMarker
      threshold = 0.7
      indicator = l2zz
      max_h_level = 3
    []
  []
[]

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
    print_level = 0
  []
  [cg]
    type = MFEMCGSolver
    preconditioner = boomeramg
    l_tol = 1e-6
    l_max_its = 500
    use_initial_guess = false
    print_level = 0
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[Postprocessors]
  [displacement_l2_norm]
    type = MFEMVectorL2Error
    variable = displacement
    function = '0 0 0'
  []
[]

[Outputs]
  [CSV]
    type = CSV
    file_base = OutputData/ElasticityAMR
  []
[]
