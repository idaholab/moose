[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [eta]
  []
[]

[Materials]
  [F]
    type = ADDerivativeParsedMaterial
    args = 'eta'
    function = 'eta+5'
    derivative_order = 3
    outputs = exodus
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
[]
