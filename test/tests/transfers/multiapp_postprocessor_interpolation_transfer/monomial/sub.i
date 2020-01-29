[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [value]
    type = FunctionValuePostprocessor
    function = 1949
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
[]
