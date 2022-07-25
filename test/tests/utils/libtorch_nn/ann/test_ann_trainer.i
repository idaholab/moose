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
    num_epochs = 2000
    num_batches = 2
    num_samples = 640
    learning_rate = 1e-1
    hidden_layers = '27 16'
    monitor_point = '0.5 0.8333333333 1.16666666666666'
    max_processes = 4
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
