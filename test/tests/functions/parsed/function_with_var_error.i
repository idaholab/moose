[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[AuxVariables/f][]

[Functions]
  [func_with_var]
    type = ParsedFunction
    vars = q
    vals = f
    value = sin(q)
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
[]
