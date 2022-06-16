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
    metric_type = STRETCH
    failure_type = ERROR
    upper_bound = 1.0
    lower_bound = 0.5
  [../]
[]

[Executioner]
  type = Steady
[]
