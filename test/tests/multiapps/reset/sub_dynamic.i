[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [time]
    type = TimePostprocessor
  []
[]

[Executioner]
  type = Transient
[]
