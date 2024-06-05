[StochasticTools]
[]

[Distributions]
  [a]
    type = TruncatedNormal
    mean = 0.8
    standard_deviation = 0.05
    lower_bound = 0.0
    upper_bound = 2.0
  []
  [b]
    type = TruncatedNormal
    mean = 0.5
    standard_deviation = 0.05
    lower_bound = 0.0
    upper_bound = 2.0
  []
  [c]
    type = TruncatedNormal
    mean = 0.5
    standard_deviation = 0.05
    lower_bound = 0.0
    upper_bound = 2.0
  []
  [d]
    type = TruncatedNormal
    mean = 0.25
    standard_deviation = 0.05
    lower_bound = 0.0
    upper_bound = 2.0
  []
  [prior_variance]
    type = Uniform
    lower_bound = 0.0
    upper_bound = 0.5
  []
[]

[Likelihood]
  [gaussianx]
    type = Gaussian
    noise = 'mcmc_reporter/noise'
    file_name = 'exp_lnx_noise_reduced.csv'
    log_likelihood=true
  []
  [gaussiany]
    type = Gaussian
    noise = 'mcmc_reporter/noise'
    file_name = 'exp_lny_noise_reduced.csv'
    log_likelihood=true
  []
  [linearx]
    type = Linear
    noise = 'mcmc_reporter/noise'
    file_name = 'exp_lnx_noise_reduced.csv'
    # log_likelihood=false
  []
  [lineary]
    type = Linear
    noise = 'mcmc_reporter/noise'
    file_name = 'exp_lny_noise_reduced.csv'
    # log_likelihood=false
  []
[]

[Samplers]
  [sample]
    type = AffineInvariantDES
    prior_distributions = 'a b c d'
    num_parallel_proposals = 20
    file_name = 'confg_predprey_reduced.csv'
    execute_on =  PRE_MULTIAPP_SETUP
    seed = 100
    initial_values = '0.8 0.5 0.5 0.25'
    previous_state = 'mcmc_reporter/inputs'
    previous_state_var = 'mcmc_reporter/variance'
    prior_variance = 'prior_variance'
    lower_bound = '0.0 0.0 0.0 0.0'
    upper_bound = '2.0 2.0 2.0 2.0'
    variance_bound = 0.5
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
  [mcmc_reporter]
    type = AIDESGPryLearner
    output_value = constant_x/reporter_transfer_x:log_x:value
    output_value1 = constant_y/reporter_transfer_y:log_y:value
    sampler = sample
    al_gp = GP_al_trainer
    gp_evaluator = GP_eval
    likelihoods = 'gaussianx gaussiany linearx lineary'
  []
[]

[Trainers]
  [GP_al_trainer]
    type = ActiveLearningGaussianProcess
    covariance_function = 'covar'
    standardize_params = 'true'
    standardize_data = 'true'
    tune_parameters = 'signal_variance length_factor'
    tuning_algorithm = 'adam'
    iter_adam = 20000
    learning_rate_adam = 0.005 # 0.0001 # 
    show_optimization_details = true
    batch_size = 100
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
    signal_variance = 10.0
    noise_variance = 1e-6 # 1e-8
    length_factor = '10.0 10.0 10.0 10.0 10.0'
  []
[]

[Executioner]
  type = Transient
  num_steps = 40
[]

[Outputs]
  csv = false
  execute_on = TIMESTEP_END
  perf_graph = true
  [out1_noAL]
    type = JSON
    execute_system_information_on = NONE
  []
  [out2_noAL]
    type = SurrogateTrainerOutput
    trainers = 'GP_al_trainer'
    # execute_on = FINAL
  []
[]
