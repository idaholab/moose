[Mesh]
  type = MFEMMesh
  file = ../mesh/beam-tet.mesh
  dim = 3
  uniform_refine = 2
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
    values = '0.0 0.0 0.0'
  []
  [pull_down]
    type = MFEMVectorBoundaryIntegratedBC
    variable = displacement
    boundary = '2'
    values = '0.0 0.0 -0.01'
  []
[]

[FunctorMaterials]
  [Rigidium]
    type = MFEMGenericConstantFunctorMaterial
    prop_names = 'lambda mu'
    prop_values = '50.0 50.0'
    block = 1
  []
  [Bendium]
    type = MFEMGenericConstantFunctorMaterial
    prop_names = 'lambda mu'
    prop_values = '1.0 1.0'
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

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
    l_max_its = 500
    l_tol = 1e-8
    print_level = 2
  []
[]

[Solver]
  type = MFEMHyprePCG
  #preconditioner = boomeramg
  l_max_its = 5000
  l_tol = 1e-8
  l_abs_tol = 0.0
  print_level = 2
[]

[Executioner]
  type = MFEMSteady
  device = "cpu"
[]

[Outputs]
  [ParaViewDataCollection]
    type = MFEMParaViewDataCollection
    file_base = OutputData/LinearElasticity
    vtk_format = ASCII
  []
[]
