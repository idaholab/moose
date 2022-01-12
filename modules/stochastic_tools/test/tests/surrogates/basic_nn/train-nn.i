[StochasticTools]
[]

[Samplers]
  [sample]
    type = CartesianProduct
    linear_space_items = '0 0.0125 8
                          0 0.0125 8
                          0 0.0125 8'
  []
  [test]
    type = CartesianProduct
    linear_space_items = '0 0.05 2
                          0 0.05 2
                          0 0.05 2'
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
    type = LibtorchSimpleNNTrainer
    sampler = sample
    response = values/g_values
    no_epochs = 200
    no_batches = 32
    no_hidden_layers = 3
    no_neurons_per_layer = '128 64 32'
    learning_rate = 0.00005
    filename = mynet.pt
  []
[]

[Surrogates]
  [surr]
    type = LibtorchSimpleNNSurrogate
    trainer = train
  []
[]

[Reporters]
  [results]
    type = EvaluateSurrogate
    model = surr
    sampler = test
    execute_on = FINAL
    parallel_type = ROOT
  []
[]

[Outputs]
  [out]
    type = SurrogateTrainerOutput
    trainers = 'train'
    execute_on = FINAL
  []
[]
