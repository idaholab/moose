[Mesh]
  [generator]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
  [subdomain1]
    type = SubdomainBoundingBoxGenerator
    input = generator
    bottom_left = '0.5 0 0'
    top_right = '1 1 0'
    block_id = 1
  []
[]

[Variables]
  [right_elemental]
    block = 1
    family = MONOMIAL
    order = CONSTANT
  []
  [right_nodal]
    block = 1
  []
[]

[ICs]
  [right_elemental]
    type = FunctionIC
    variable = right_elemental
    function = 2*y
  []
  [right_nodal]
    type = FunctionIC
    variable = right_nodal
    function = 3*y
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
