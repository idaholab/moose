[Mesh]
  file = sq-2blk.e
  uniform_refine = 1
[]

[Variables]
  [u]
    type = MooseVariableFVReal
  []
[]

[FVICs]
  [ic_u_1]
    type = FVConstantIC
    variable = u
    value = 6.25
    block = '1'
  []
  [ic_u_2]
    type = FVConstantIC
    variable = u
    value = 9.99
    block = '2'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]
