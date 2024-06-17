[StochasticTools]
[]

[Distributions]
  [a]
    # type = TruncatedNormal
    # mean = 0.95
    # standard_deviation = 0.05
    type = Uniform
    lower_bound = 0.65
    upper_bound = 1.35
  []
  [b]
    # type = TruncatedNormal
    # mean = 0.5
    # standard_deviation = 0.05
    type = Uniform
    lower_bound = 0.15
    upper_bound = 0.65
  []
  [c]
    # type = TruncatedNormal
    # mean = 0.5
    # standard_deviation = 0.05
    type = Uniform
    lower_bound = 0.15
    upper_bound = 0.65
  []
  [d]
    # type = TruncatedNormal
    # mean = 0.2
    # standard_deviation = 0.05
    type = Uniform
    lower_bound = 0.0
    upper_bound = 0.3
  []
  [prior_variance]
    type = Uniform
    lower_bound = 0.0
    upper_bound = 0.25
  []
[]

[Likelihood]
  [gaussianx]
    type = Gaussian
    noise = 'conditional/noise'
    # noise = 'noise_specified/noise_specified'
    file_name = 'exp_lnx_noise.csv'
    # log_likelihood=true
  []
  [gaussiany]
    type = Gaussian
    noise = 'conditional/noise'
    # noise = 'noise_specified/noise_specified'
    file_name = 'exp_lny_noise.csv'
    # log_likelihood=true
  []
[]

[Samplers]
  [sample]
    type = BayesianGPrySampler
    prior_distributions = 'a b c d'
    sorted_indices = 'conditional/sorted_indices'
    num_parallel_proposals = 48
    file_name = 'confg_predprey.csv'
    execute_on = PRE_MULTIAPP_SETUP
    num_tries = 5000
    seed = 100
    initial_values = '0.8 0.5 0.5 0.2'
    prior_variance = 'prior_variance'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = predprey_exact.i
    sampler = sample
  []
[]

[Transfers]
  [reporter_transfer_x]
    type = SamplerReporterTransfer
    from_reporter = 'log_x/value'
    stochastic_reporter = 'constant_x'
    from_multi_app = sub
    sampler = sample
  []
  [reporter_transfer_y]
    type = SamplerReporterTransfer
    from_reporter = 'log_y/value'
    stochastic_reporter = 'constant_y'
    from_multi_app = sub
    sampler = sample
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = sample
    param_names = 'a b c d end_time'
  []
[]

[Reporters]
  [constant_x]
    type = StochasticReporter
  []
  [constant_y]
    type = StochasticReporter
  []
  # [noise_specified]
  #   type = ConstantReporter
  #   real_names = 'noise_specified'
  #   real_values = '0.25'
  # []
  [conditional]
    type = BayesianGPryLearner
    output_value = constant_x/reporter_transfer_x:log_x:value
    output_value1 = constant_y/reporter_transfer_y:log_y:value
    sampler = sample
    al_gp = GP_al_trainer
    gp_evaluator = GP_eval
    likelihoods = 'gaussianx gaussiany'
  []
[]

[Trainers]
  [GP_al_trainer]
    type = ActiveLearningGaussianProcess
    covariance_function = 'covar'
    standardize_params = 'true'
    standardize_data = 'true'
    tune_parameters = 'signal_variance noise_variance length_factor'
    tuning_algorithm = 'adam'
    iter_adam = 10000
    learning_rate_adam = 0.001 # 0.0001 # 
    show_optimization_details = true
    batch_size = 5000
  []
[]

[Surrogates]
  [GP_eval]
    type = GaussianProcess
    trainer = GP_al_trainer
  []
[]

[Covariance]
  [covar]
    type = SquaredExponentialCovariance
    signal_variance = 4.0
    noise_variance = 0.01
    length_factor = '4.0 4.0 4.0 4.0 4.0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 50
[]

[Outputs]
  csv = false
  execute_on = TIMESTEP_END
  perf_graph = true
  [out1_Targeted]  # 
    type = JSON
    execute_system_information_on = NONE
  []
  [out2_Targeted] # 
    type = SurrogateTrainerOutput
    trainers = 'GP_al_trainer'
    # execute_on = FINAL
  []
[]
