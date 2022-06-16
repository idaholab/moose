[Mesh]
  file = Quad.e
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]
  [./top]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[UserObjects]
  [./elem_quality_check]
    type = ElementQualityChecker
    metric_type = DIAGONAL
    failure_type = WARNING
  [../]
[]

[Executioner]
  type = Steady
[]
