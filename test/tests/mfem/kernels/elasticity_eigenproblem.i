# Elasticity eigenproblem for a two-material cantilever beam, clamped on boundary 1.

[Mesh]
  type = MFEMFileMesh
  file = ../mesh/beam-tet.mesh
  serial_refine = 1
  parallel_refine = 1
[]

[Problem]
  type = MFEMEigenproblem
  num_modes = 5
[]

[FESpaces]
  [H1FESpace]
    type = MFEMVectorFESpace
    fec_type = H1
    fec_order = FIRST
    ordering = VDIM
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

[Solvers]
  [boomeramg]
    type = MFEMHypreBoomerAMG
    fespace = H1FESpace
    vector_treatment = by_component
    print_level = 0
  []
  [LOBPCG]
    type = MFEMHypreLOBPCG
    preconditioner = boomeramg
    print_level = 0
    l_tol = 1e-8
    l_max_its = 100
    random_seed = 66
  []
[]

[Executioner]
  type = MFEMSteady
  device = cpu
[]

[VectorPostprocessors]
  [eigenvalues]
    type = MFEMEigenvaluesPostprocessor
  []
[]

[Outputs]
  [ReportedPostprocessors]
    type = CSV
    file_base = OutputData/ElasticityEigenproblem
  []
[]
