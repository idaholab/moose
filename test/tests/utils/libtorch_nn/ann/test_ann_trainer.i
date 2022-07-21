[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[VectorPostprocessors]
  [test_trainer]
    type = LibtorchArtificialNeuralNetTrainerTest
    optimizer_type = rmsprop
    num_epochs = 1000
    num_batches = 2
    learning_rate = 1e-3
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
