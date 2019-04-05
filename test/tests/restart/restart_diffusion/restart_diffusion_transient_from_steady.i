[Mesh]
  file = square.e
[]

[Problem]
  restart_file_base = steady_out_cp/0001
  skip_additional_restart_data = true
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'bodyforce ie'

  [./bodyforce]
    type = BodyForce
    variable = u
    value = 10.0
  [../]

  [./ie]
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
    boundary = 2
    value = 1
  [../]
[]

[Postprocessors]
  [./unorm]
    type = ElementL2Norm
    variable = u
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  # Reset the start_time here
  start_time = 0.0
  num_steps = 10
  dt = .1
[]

[Outputs]
  exodus = true
[]
