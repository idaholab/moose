[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 20
  ny = 20
  nz = 20
  xmin = 0.0
  xmax = 10.0
  ymin = 0.0
  ymax = 10.0
  zmin = 0.0
  zmax = 10.0
[]

[Variables]
  [./c]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[ICs]
  [./c]
    variable = c
    type = SmoothCircleIC
    x1 = 5.0
    y1 = 5.0
    z1 = 5.0
    radius = 4.0
    invalue = 1.0
    outvalue = 0.0
    int_width = 1.0
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
  exodus = true
[]
