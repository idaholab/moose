[Mesh]
  file = steady_out.e
[]

[Variables]
  [./u_new]
    order = FIRST
    family = LAGRANGE

    # Testing that we can load a solution from a different variable name
    initial_from_file_var = u
    initial_from_file_timestep = 2
  [../]
[]

[Kernels]
  active = 'bodyforce ie'

  [./bodyforce]
    type = BodyForce
    variable = u_new
    value = 10.0
  [../]

  [./ie]
    type = TimeDerivative
    variable = u_new
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u_new
    boundary = 3
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u_new
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  start_time = 0.0
  num_steps = 10
  dt = .1
[]

[Outputs]
  exodus = true
[]
