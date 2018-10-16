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
  [../]
[]

[ICs]
  [./A]
    type = VectorConstantIC
    variable = A
    x_value = 2
    y_value = 3
    z_value = 4
  [../]
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
[]

[Outputs]
  exodus = true
[]
