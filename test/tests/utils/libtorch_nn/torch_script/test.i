[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[VectorPostprocessors]
  [test]
    type = LibtorchTorchScriptNeuralNetTest
    filename = "my_net.pt"
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
