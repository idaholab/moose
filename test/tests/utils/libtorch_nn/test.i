[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[UserObjects]
  [test]
    type = LibtorchSimpleNeuralNetTest
    activation_functions = 'relu relu'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
