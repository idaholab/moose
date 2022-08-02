[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[VectorPostprocessors]
  [test_trainer]
    type = LibtorchArtificialNeuralNetTrainerTest
    optimizer_type = adam
    num_epochs = 1000
    num_batches = 20
    num_samples = 1000
    learning_rate = 1e-4
    hidden_layers = '108 54 16'
    monitor_point = '0.5 0.8333333333 1.16666666666666'
    max_processes = 32
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
