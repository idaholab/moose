[Mesh]
  file = exodus_refined_restart_1.e
  # Restart relies on the ExodusII_IO::copy_nodal_solution()
  # functionality, which only works with ReplicatedMesh.
  parallel_type = replicated
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    initial_from_file_var = u
    initial_from_file_timestep = 2
  [../]
  [v]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [./bodyforce_u]
    type = BodyForce
    variable = u
    value = 10.0
  [../]

  [./ie_u]
    type = TimeDerivative
    variable = u
  [../]

  [./bodyforce_v]
    type = BodyForce
    variable = v
    value = 10.0
  [../]

  [./ie_v]
    type = TimeDerivative
    variable = v
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 3
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]

  [./top]
    type = DirichletBC
    variable = v
    boundary = 0
    value = 0
  [../]

  [./bottom]
    type = DirichletBC
    variable = v
    boundary = 2
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
  file_base = exodus_refined_restart_2
  exodus = true
[]
