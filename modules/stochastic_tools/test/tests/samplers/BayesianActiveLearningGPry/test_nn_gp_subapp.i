[StochasticTools]
[]

[Distributions]
  [left]
    type = Normal
    mean = 0.0
    standard_deviation = 1.0
  []
  [right]
    type = Normal
    mean = 0.0
    standard_deviation = 1.0
  []
  [variance]
    type = Uniform
    lower_bound = 0.0
    upper_bound = 0.5
  []
[]

[Likelihood]
  [gaussian]
    type = Gaussian
    noise = 'mcmc_reporter/noise'
    file_name = 'exp_0_05.csv'
    log_likelihood = false
  []
[]

[Samplers]
  [sample]
    type = BayesianGPrySampler
    prior_distributions = 'left right'
    sorted_indices = 'conditional/sorted_indices'
    num_parallel_proposals = 50
    file_name = 'confg.csv'
    execute_on = PRE_MULTIAPP_SETUP
    num_tries = 1000
    seed = 2547
    initial_values = '0.1 0.1'
    previous_state = 'mcmc_reporter/inputs'
    previous_state_var = 'mcmc_reporter/variance'
    prior_variance = 'variance'
  []
  # [test]
  #   type = MonteCarlo
  #   distributions = 'normal normal'
  #   num_rows = 1000
  #   seed = 100
  #   execute_on = PRE_MULTIAPP_SETUP
  # []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
  []
  # [sub_test]
  #   type = SamplerFullSolveMultiApp
  #   input_files = sub.i
  #   sampler = test
  # []
[]

[Transfers]
  [reporter_transfer]
    type = SamplerReporterTransfer
    from_reporter = 'average/value'
    stochastic_reporter = 'constant'
    from_multi_app = sub
    sampler = sample
  []
  # [reporter_transfer_test]
  #   type = SamplerReporterTransfer
  #   from_reporter = 'average/value'
  #   stochastic_reporter = 'constant_test'
  #   from_multi_app = sub_test
  #   sampler = test
  # []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = sample
    param_names = 'left_bc right_bc'
  []
  # [cmdline_test]
  #   type = MultiAppSamplerControl
  #   multi_app = sub_test
  #   sampler = test
  #   param_names = 'left_bc right_bc'
  # []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  # [constant_test]
  #   type = StochasticReporter
  # []
  [conditional]
    type = BayesianGPryLearner
    output_value = constant/reporter_transfer:average:value
    sampler = sample
    al_nn = train
    nn_evaluator = surr
    al_gp = GP_al_trainer
    gp_evaluator = GP_eval
  []
  # [results]
  #   type = EvaluateSurrogate
  #   model = surr
  #   sampler = test
  #   execute_on = FINAL
  #   parallel_type = ROOT
  # []
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
  [GP_al_trainer]
    type = ActiveLearningGaussianProcess
    covariance_function = 'covar'
    standardize_params = 'true'
    standardize_data = 'true'
    tune_parameters = 'signal_variance length_factor'
    tuning_algorithm = 'adam'
    iter_adam = 5000
    learning_rate_adam = 0.001
    show_optimization_details = true
    batch_size = 100
  []
[]

[Surrogates]
  [surr]
    type = LibtorchANNSurrogate
    trainer = train
    classify = true
  []
  [GP_eval]
    type = GaussianProcess
    trainer = GP_al_trainer
  []
[]

[Covariance]
  [covar]
    type = SquaredExponentialCovariance
    signal_variance = 1.0
    noise_variance = 1e-4
    length_factor = '1.0 1.0'
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
