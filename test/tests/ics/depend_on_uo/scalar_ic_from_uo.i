# This test sets an initial condition of a scalar variable from an user object

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 8
  ny = 8

  # We are testing geometric ghosted functors
  # so we have to use distributed mesh
  parallel_type = distributed
[]

[Variables]
  [./u]
  [../]

  [./a]
    family = SCALAR
    order = FIRST
  [../]
[]

[ICs]
  [./ghost_ic]
    type = ScalarUOIC
    variable = a
    user_object = scalar_uo
  [../]
[]

[UserObjects]
  [./scalar_uo]
    type = MTUserObject
  [../]
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  show = 'a'
[]

[Problem]
  kernel_coverage_check = false
[]
