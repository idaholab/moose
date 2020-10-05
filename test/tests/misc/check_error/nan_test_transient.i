[Mesh]
  [./square]
    type = GeneratedMeshGenerator
    nx = 2
    ny = 2
    dim = 2
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./nan]
    type = NanKernel
    variable = u
    timestep_to_nan = 2
  [../]
[]

[BCs]
  [./right]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 1
  [../]
[]

[Executioner]
  type = Transient

  solve_type = 'PJFNK'
  dt = 1
  num_steps = 2
[]
