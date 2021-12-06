[StochasticTools]
[]

[Samplers]
  [sample]
    type = CartesianProduct
    linear_space_items = '0 0.05 20
                          0 0.05 20
                          0 0.05 20'
  []
[]

[VectorPostprocessors]
  [values]
    type = GFunction
    sampler = sample
    q_vector = '0 0 0'
    execute_on = INITIAL
    outputs = none
  []
[]

[Trainers]
  [train]
    type = BasicNNTrainer
    sampler = sample
    response = values/g_values
    no_epochs = 20000
    no_batches = 40
    no_hidden_layers = 3
    no_neurons_per_layer = '48 24 12'
    learning_rate = 0.0002
  []
[]

[Outputs]
  [out]
    type = SurrogateTrainerOutput
    trainers = 'train'
    execute_on = FINAL
  []
[]
