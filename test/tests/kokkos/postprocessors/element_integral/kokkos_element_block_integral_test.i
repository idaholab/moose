[Mesh]
  file = rectangle.e
[]

[Variables]
  active = 'u'

  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  active = 'diff'

  [diff]
    type = KokkosDiffusion
    variable = u
  []
[]

[BCs]
  active = 'left right'

  [left]
    type = KokkosDirichletBC
    variable = u
    boundary = 1
    value = 0
  []

  [right]
    type = KokkosDirichletBC
    variable = u
    boundary = 2
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Postprocessors]
  [integral_left]
    type = KokkosElementIntegralVariablePostprocessor
    variable = u
    block = 1
  []

  [integral_right]
    type = KokkosElementIntegralVariablePostprocessor
    variable = u
    block = 2
  []

  [integral_all]
    type = KokkosElementIntegralVariablePostprocessor
    variable = u
  []
[]

[Outputs]
  csv = true
[]
