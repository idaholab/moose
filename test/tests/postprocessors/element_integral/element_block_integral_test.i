[Mesh]
  file = rectangle.e
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Postprocessors]
  [./integral_left]
    type = ElementIntegralVariablePostprocessor
    variable = u
    block = 1
  [../]

  [./integral_right]
    type = ElementIntegralVariablePostprocessor
    variable = u
    block = 2
  [../]

  [./integral_all]
    type = ElementIntegralVariablePostprocessor
    variable = u
  [../]
[]

[Outputs]
  file_base = out_block
  exodus = false
  csv = true
[]
