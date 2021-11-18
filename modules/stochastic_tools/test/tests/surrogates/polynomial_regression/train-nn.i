[StochasticTools]
[]

[Samplers]
  [sample]
    type = CartesianProduct
    linear_space_items = '0 0.01 10
                          0 0.01 10
                          0 0.01 10'
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
    no_epochs = 60
    no_batches = 20
    no_hidden_layers = 3
    no_neurons_per_layer = '16 8 4'
    learning_rate = 0.01
  []
[]

[Outputs]
  [out]
    type = SurrogateTrainerOutput
    trainers = 'train'
    execute_on = FINAL
  []
[]
