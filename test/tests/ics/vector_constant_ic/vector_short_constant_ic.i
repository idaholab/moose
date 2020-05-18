[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Variables]
  [./A]
    family = LAGRANGE_VEC
    order = FIRST
    initial_condition = '2 3 4'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
