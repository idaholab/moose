[StochasticTools]
[]

[Distributions]
  [left]
    type = TruncatedNormal
    mean = 0.0
    standard_deviation = 1.0
    # type = Uniform
    lower_bound = -1.0
    upper_bound = 1.0
  []
  [right]
    type = TruncatedNormal
    mean = 0.0
    standard_deviation = 1.0
    # type = Uniform
    lower_bound = -1.0
    upper_bound = 1.0
  []
  [variance]
    type = Uniform
    lower_bound = 0.0
    upper_bound = 0.1
  []
[]

[Likelihood]
  [gaussian]
    type = Gaussian
    noise = 'conditional/noise'
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
    prior_variance = 'variance'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
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
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = sample
    param_names = 'left_bc right_bc mesh1'
  []
[]

[Reporters]
  [constant]
    type = StochasticReporter
  []
  [conditional]
    type = BayesianGPryLearner
    output_value = constant/reporter_transfer:average:value
    sampler = sample
    al_nn = train
    nn_evaluator = surr
    al_gp = GP_al_trainer
    gp_evaluator = GP_eval
    likelihoods = 'gaussian'
  []
[]

[Trainers]
  [train]
    type = ActiveLearningLibtorchNN
    num_epochs = 10
    num_batches = 250
    num_neurons_per_layer = '40 40 40'
    learning_rate = 0.0001
    nn_filename = classifier_net.pt
    print_epoch_loss = 10
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
    iter_adam = 1000
    learning_rate_adam = 0.0005
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
    noise_variance = 0.0 # 1e-8
    length_factor = '1.0 1.0 1.0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 75
[]

[Outputs]
  csv = false
  execute_on = TIMESTEP_END
  perf_graph = true
  [out]
    type = SurrogateTrainerOutput
    trainers = 'train'
    execute_on = FINAL
  []
  [out1]
    type = JSON
    execute_system_information_on = NONE
  []
  [out2]
    type = SurrogateTrainerOutput
    trainers = 'GP_al_trainer'
    # execute_on = FINAL
  []
[]
