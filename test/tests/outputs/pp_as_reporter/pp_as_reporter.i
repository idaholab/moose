[Mesh/gen]
  type = GeneratedMeshGenerator
  dim = 1
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors/data]
  type = FunctionValuePostprocessor
  function = 1980
[]

[Outputs]
  [out]
    type = JSON
    postprocessors_as_reporters = true
  []
[]
