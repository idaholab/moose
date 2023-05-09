[StochasticTools]
[]

[Samplers]
  [sample]
    type = CartesianProduct
    linear_space_items = '0 0.0125 5
                          0 0.0125 5
                          0 0.0125 5'
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
    type = LibtorchANNTrainer
    sampler = sample
    response = values/g_values
    num_epochs = 40
    num_batches = 10
    num_neurons_per_layer = '64 32'
    learning_rate = 0.001
    nn_filename = mynet_tne.pt
    read_from_file = false
    print_epoch_loss = 10
    max_processes = 1
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
    sampler = test
    execute_on = FINAL
    parallel_type = ROOT
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
