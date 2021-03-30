[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
  elem_type = QUAD4
  nx = 8
  ny = 8
[]

[Variables]
  [u]
    initial_condition = 1
  []
  [T]
    initial_condition = 1
  []
[]

[AuxVariables]
  [power][]
[]

[Kernels]
  [./diff]
    type = DiffMKernel
    variable = u
    mat_prop = diffusion
    offset = 0.0
  [../]

  [./rhs]
    type = CoefReaction
    variable = u
    coefficient = -1.0
    extra_vector_tags = 'eigen'
  [../]

  [./diff_T]
    type = CoefDiffusion
    variable = T
    coef = 1e30
  [../]
  [./src_T]
    type = CoupledForce
    variable = T
    v = power
    coef = 1e30
  [../]
[]

[AuxKernels]
  [./power_ak]
    type = NormalizationAux
    variable = power
    source_variable = u
    normalization = unorm
    # this coefficient will affect the eigenvalue.
    normal_factor = 10
    execute_on = linear
  [../]
[]

[BCs]
  [./homogeneous]
    type = DirichletBC
    variable = u
    boundary = '0 1 2 3'
    value = 0
  [../]

  [./eigenU]
    type = EigenDirichletBC
    variable = u
    boundary = '0 1 2 3'
  [../]

  [./homogeneousT]
    type = DirichletBC
    variable = T
    boundary = '0 1 2 3'
    value = 0
  [../]

  [./eigenT]
    type = EigenDirichletBC
    variable = T
    boundary = '0 1 2 3'
  [../]
[]

[Materials]
  [./dc]
    type = VarCouplingMaterial
    var = T
    block = 0
    base = 1.0
    coef = 1.0
  [../]
[]

[Executioner]
  type = Eigenvalue
  solve_type = PJFNK
  automatic_scaling = true
  petsc_options = '-pc_svd_monitor'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'svd'
  verbose = true
  resid_vs_jac_scaling_param = 1
[]

[Postprocessors]
  [./unorm]
    type = ElementIntegralVariablePostprocessor
    variable = u
    execute_on = linear
  [../]
[]

[VectorPostprocessors]
  [./eigenvalues]
    type = Eigenvalues
    execute_on = 'timestep_end'
  [../]
[]

[Outputs]
  exodus = true
  csv = true
  execute_on = 'timestep_end'
[]
