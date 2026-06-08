[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Variables]
  [u]
  []
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [from_parent]
    type = Receiver
  []
  [to_parent]
    type = ParsedPostprocessor
    expression = 'from_parent + 5'
    pp_names = 'from_parent'
  []
[]

[Executioner]
  type = Steady
[]
