[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 10
  nz = 0
  xmin = 0
  xmax = 50
  ymin = 0
  ymax = 25
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[Variables]
  [./c]
    order = THIRD
    family = HERMITE
    [./InitialCondition]
      type = GradientBoxIC
      x1 = 15.0
      x2 = 35.0
      y1 = 0.0
      y2 = 25.0
      grad_direction = 0
      grad_sign = 1.0
      mn_value = -1.0
      mx_value = 1.0
    [../]
  [../]
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 0
[]

[Outputs]
  exodus = true
[]
