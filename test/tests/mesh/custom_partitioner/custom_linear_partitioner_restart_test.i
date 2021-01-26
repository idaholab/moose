[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2

    nx = 10
    ny = 10

    xmin = 0.0
    xmax = 1.0

    ymin = 0.0
    ymax = 10.0
  []

  [./Partitioner]
    type = LibmeshPartitioner
    partitioner = linear
  [../]
  parallel_type = replicated
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

[Executioner]
  type = Transient

  solve_type = 'PJFNK'

  start_time = 0.0
  num_steps = 10
  dt = .1
[]

[Outputs]
  file_base =  custom_linear_partitioner_restart_test_out
  exodus = true
[]
