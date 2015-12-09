[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = CoefDiffusion
    variable = u
    coef = 0.1
  [../]
  [./time]
    type = TimeDerivative
    variable = u
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
  [./right2]
    type = FunctionDirichletBC
    variable = u
    boundary = right
    function = (y*(t-1))+1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 20
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[Controls]
  [./period0]
    type = TimePeriod
    disable_objects = 'BCs::right2'
    start_time = '0'
    end_time = '0.95'
    execute_on = 'initial timestep_begin'
  [../]

  [./period2]
    type = TimePeriod
    disable_objects = 'BCs::right'
    start_time = '1'
    execute_on = 'initial timestep_begin'
  [../]
[]
