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
    type = BayesianGPrySampler
    distributions = 'uniform uniform uniform uniform uniform'
    num_iterations = 20
    num_samples = 16
    num_tries = 1000
    # nn_name = surr
  []
  [test]
    type = MonteCarlo
    distributions = 'uniform uniform uniform uniform uniform'
    num_rows = 1000
    seed = 100
  []
[]

[VectorPostprocessors]
  [values]
    type = nDRosenbrock
    sampler = sample
    # execute_on = INITIAL
  []
  [test_values]
    type = nDRosenbrock
    sampler = test
    execute_on = FINAL
    limiting_value = 0.0
    classify = true
  []
[]

[Reporters]
  [conditional]
    type = BayesianGPryLearner
    output_value = values/g_values
    sampler = sample
    al_nn = train
  []
  [results]
    type = EvaluateSurrogate
    model = surr
    sampler = test
    execute_on = FINAL
    parallel_type = ROOT
  []
[]

[Trainers]
  [train]
    type = ActiveLearningLibtorchNN
    num_epochs = 15000
    num_batches = 250
    num_neurons_per_layer = '40 40 40'
    learning_rate = 0.001
    nn_filename = classifier_net.pt
    print_epoch_loss = 1000
    max_processes = 1
    standardize_input = true
    standardize_output = false
    classify = true
    activation_function = 'relu'
  []
[]

[Surrogates]
  [surr]
    type = LibtorchANNSurrogate
    trainer = train
    classify = true
  []
[]

[Executioner]
  type = Transient
  num_steps = 20
[]

[Outputs]
  csv = true
  execute_on = FINAL
  [out]
    type = SurrogateTrainerOutput
    trainers = 'train'
    execute_on = FINAL
  []
[]
