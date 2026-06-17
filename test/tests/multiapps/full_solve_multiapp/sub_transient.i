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
    expression = 'from_parent + t'
    pp_names = 'from_parent'
    use_t = true
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 1
[]
