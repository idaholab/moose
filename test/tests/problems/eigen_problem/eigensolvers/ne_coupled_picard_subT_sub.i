[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 10
  ymin = 0
  ymax = 10
  elem_type = QUAD4
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxVariables]
  [./T]
    order = FIRST
    family = LAGRANGE
  [../]
  [./power]
    order = FIRST
    family = LAGRANGE
  [../]
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
[]

[AuxKernels]
  [./power_ak]
    type = NormalizationAux
    variable = power
    source_variable = u
    normalization = unorm
    normal_factor = 10
    execute_on = timestep_end
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
  nl_abs_tol = 1e-8
  nl_rel_tol = 1e-6
[]

[Postprocessors]
  [./power]
    type = ElementIntegralVariablePostprocessor
    variable = power
    execute_on = timestep_end
  [../]
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
  csv = true
  exodus = true
  execute_on = 'timestep_end'
[]
