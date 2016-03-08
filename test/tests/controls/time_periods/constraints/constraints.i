[Mesh]
  type = FileMesh
  file = constraints.e
[../]

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
    boundary = 1
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 1
  [../]
[]

[Constraints]
  [./complete]
    type = TiedValueConstraint
    variable = u
    slave = 2
    master = 3
    master_variable = u
  [../]
  [./lower]
    type = TiedValueConstraint
    variable = u
    slave = inside_right_lower
    master = inside_left_lower
    master_variable = u
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 40
  dt = 0.1
  solve_type = PJFNK
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]

[Controls]
  [./constraints]
    type = TimePeriod
    disable_objects = 'lower complete'
    start_time      = '0.0   2.0'
    end_time        = '2.0   4.0'
    execute_on = 'initial timestep_begin'
  [../]
[]
