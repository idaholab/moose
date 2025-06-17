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
    vector_coefficient = '0.0 0.0 0.0'
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
  [RigidiumWeightDensity]
    type = MFEMGenericFunctorVectorMaterial
    prop_names = 'gravitational_force_density'
    prop_values = '{0.0 0.0 -1e-2}'
    block = 1
  []
  [BendiumWeightDensity]
    type = MFEMGenericFunctorVectorMaterial
    prop_names = 'gravitational_force_density'
    prop_values = '{0.0 0.0 -5e-3}'
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
  [gravity]
    type = MFEMVectorDomainLFKernel
    variable = displacement
    vector_coefficient = gravitational_force_density
  []
[]

[Preconditioner]
  [boomeramg]
    type = MFEMHypreBoomerAMG
    fespace = H1FESpace
    l_max_its = 20
    l_tol = 1e-5
    print_level = 2
  []
[]

[Solver]
  type = MFEMHyprePCG
  preconditioner = boomeramg
  l_max_its = 100
  l_tol = 1e-4
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
    file_base = OutputData/Gravity
    vtk_format = ASCII
  []
[]
