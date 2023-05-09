[StochasticTools]
[]

[Samplers]
  [train_sample]
    type = CartesianProduct
    linear_space_items = '0 0.0125 5
                          0 0.0125 5
                          0 0.0125 5'
  []
  [test_sample]
    type = CartesianProduct
    linear_space_items = '0 0.05 2
                          0 0.05 2
                          0 0.05 2'
  []
[]

[VectorPostprocessors]
  [values]
    type = GFunction
    sampler = train_sample
    q_vector = '0 0 0'
    execute_on = INITIAL
    outputs = none
  []
[]

[Trainers]
  [train]
    type = LibtorchANNTrainer
    sampler = train_sample
    response = values/g_values
    num_epochs = 40
    num_batches = 10
    num_neurons_per_layer = '64 32'
    learning_rate = 0.001
    nn_filename = mynet.pt
    read_from_file = false
    print_epoch_loss = 10
  []
[]

[Surrogates]
  [surr]
    type = LibtorchANNSurrogate
    trainer = train
  []
[]

[Reporters]
  [results]
    type = EvaluateSurrogate
    model = surr
    sampler = test_sample
    execute_on = FINAL
    parallel_type = ROOT
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
