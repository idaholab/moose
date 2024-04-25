[StochasticTools]
[]

[Distributions]
  [normal]
    type = Normal
    mean = 0.0
    standard_deviation = 1.0
  [] 
[]

[Samplers]
  [sample]
    type = BayesianGPrySampler
    distributions = 'normal normal'
    num_iterations = 10
    num_samples = 50
    num_tries = 1000
    # nn_name = surr
    model = 'surr'
    execute_on = PRE_MULTIAPP_SETUP
  []
  [test]
    type = MonteCarlo
    distributions = 'normal normal'
    num_rows = 1000
    seed = 100
    execute_on = PRE_MULTIAPP_SETUP
  []
[]


[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
  []
  [sub_test]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = test
  []
[]

[Transfers]
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'average/value'
    stochastic_reporter = 'constant'
    from_multi_app = sub
    sampler = sample
  []
  [reporter_transfer_test]
    type = SamplerReporterTransfer
    from_reporter = 'average/value'
    stochastic_reporter = 'constant_test'
    from_multi_app = sub_test
    sampler = test
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = sample
    param_names = 'left_bc right_bc'
  []
  [cmdline_test]
    type = MultiAppSamplerControl
    multi_app = sub_test
    sampler = test
    param_names = 'left_bc right_bc'
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [constant_test]
    type = StochasticReporter
  []
  [conditional]
    type = BayesianGPryLearner
    output_value = constant/reporter_transfer:average:value
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
  num_steps = 10
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
