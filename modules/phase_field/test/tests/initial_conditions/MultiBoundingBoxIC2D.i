[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
[]

[Variables]
  [./c1]
    order = FIRST
    family = LAGRANGE
  [../]
  [./c2]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./c1]
    type = MultiBoundingBoxIC
    corners          = '0.1 0.4 0   0.8 0.5 0   0.3 0.7 0'
    opposite_corners = '0.2 0.6 0   0.6 0.9 0   0.4 0.5 0'
    inside = '1.0'
    outside = 0.1
    variable = c1
  [../]
  [./c2]
    type = MultiBoundingBoxIC
    corners          = '0.1 0.4 0   0.8 0.5 0   0.3 0.6 0'
    opposite_corners = '0.2 0.6 0   0.4 0.9 0   0.5 0.8 0'
    inside = '1.0 2.0 3.0'
    outside = 0.1
    variable = c2
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  exodus = true
[]
