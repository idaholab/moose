[StochasticTools]
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = -7.0
    upper_bound = 7.0
  [] 
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 1000
    distributions = 'uniform uniform uniform uniform uniform'
  []
  [test]
    type = MonteCarlo
    num_rows = 1000
    distributions = 'uniform uniform uniform uniform uniform'
  []
[]

[VectorPostprocessors]
  [values]
    type = nDRosenbrock
    sampler = sample
    execute_on = INITIAL
    # outputs = none
    classify = true
  []
  [test_values]
    type = nDRosenbrock
    sampler = test
    execute_on = INITIAL
    # outputs = none
    classify = true
  []
[]

[Trainers]
  [train]
    type = LibtorchANNTrainer
    sampler = sample
    response = values/g_values
    num_epochs = 5000
    num_batches = 100
    num_neurons_per_layer = '15 15'
    learning_rate = 0.001
    nn_filename = mynet_tne.pt
    read_from_file = false
    print_epoch_loss = 50
    max_processes = 1
    standardize_input = true
    standardize_output = false
    classify = true
  []
[]

[Surrogates]
  [surr]
    type = LibtorchANNSurrogate
    trainer = train
    classify = true
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
