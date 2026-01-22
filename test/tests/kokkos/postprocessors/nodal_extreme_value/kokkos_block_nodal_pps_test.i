[Mesh]
  file = rect-2blk.e
[]

[Variables]
  [u]
    order = FIRST
    family = LAGRANGE
    block = 1
  []
  [v]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  [diff_u]
    type = KokkosDiffusion
    variable = u
  []
  [diff_v]
    type = KokkosDiffusion
    variable = v
  []
[]

[BCs]
  [left_u]
    type = KokkosDirichletBC
    variable = u
    boundary = 6
    value = 0
  []
  [right_u]
    type = KokkosNeumannBC
    variable = u
    boundary = 8
    value = 4
  []
  [left_v]
    type = KokkosDirichletBC
    variable = v
    boundary = 6
    value = 1
  []
  [right_v]
    type = KokkosDirichletBC
    variable = v
    boundary = 3
    value = 6
  []
[]

[Postprocessors]
  # This test demonstrates that you can have a block restricted NodalPostprocessor
  [restricted_max]
    type = KokkosNodalExtremeValue
    variable = v
    block = 1 # Block restricted
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
