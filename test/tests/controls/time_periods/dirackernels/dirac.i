[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  uniform_refine = 2
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.5
  [../]
  [./time]
    type = TimeDerivative
    variable = u
  [../]
[]

[DiracKernels]
  [./point_source]
    type = ConstantPointSource
    variable = u
    value = 1
    point = '0.25 0.25'
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 10
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[Controls]
  [./point_source]
    type = TimePeriod
    disable_objects = 'DiracKernel::point_source'
    start_time = '0.15'
    end_time = '0.35'
    execute_on = 'initial timestep_begin'
  [../]
[]
