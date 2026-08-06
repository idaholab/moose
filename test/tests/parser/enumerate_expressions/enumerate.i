[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Problem]
  solve = false
[]

[Reporters]
  [names]
    type = ConstantReporter
    string_vector_names = 'blocks'
    string_vector_values = 'block0 ${enumerate block1 block3} block4'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
