[Mesh]
  file = exodus_refined_restart_1.e
  uniform_refine = 1
  # Restart relies on the ExodusII_IO::copy_nodal_solution()
  # functionality, which only works with SerialMesh.
  distribution = serial
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
    initial_from_file_var = u
    initial_from_file_timestep = 2
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
  active = 'left right'

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

[Executioner]
  type = Transient

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  start_time = 0.0
  num_steps = 10
  dt = .1
[]

[Output]
  file_base = exodus_refined_refined_restart_2
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]


