[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 25
  ny = 25
  xmax = 30
  ymax = 30
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = THIRD
    family = HERMITE
  [../]
[]

[ICs]
  [./c]
    type = SmoothCircleIC
    variable = c
    x1 = 15.0
    y1 = 15.0
    radius = 8.0
    invalue = 1.0
    outvalue = -0.8
    int_width = 8.0
    profile = TANH
  [../]
[]

[BCs]
  [./Periodic]
    [./all]
      auto_direction = 'x y'
    [../]
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Outputs]
  exodus = false
  [./out]
    type = Exodus
    refinements = 1
  [../]
[]
