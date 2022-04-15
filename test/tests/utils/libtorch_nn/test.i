[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects]
  [test]
    type = LibtorchArtificialNeuralNetTest
    activation_functions = 'relu relu'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
