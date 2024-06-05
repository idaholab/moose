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
#   [prior_variance]
#     type = Uniform
#     lower_bound = 0.0
#     upper_bound = 0.5
#   []
[]

[Likelihood]
  [gaussianx]
    type = Gaussian
    # noise = 'mcmc_reporter/noise'
    noise = 'noise_specified/noise_specified'
    file_name = 'exp_lnx_noise_reduced.csv'
    log_likelihood=true
  []
  [gaussiany]
    type = Gaussian
    # noise = 'mcmc_reporter/noise'
    noise = 'noise_specified/noise_specified'
    file_name = 'exp_lny_noise_reduced.csv'
    log_likelihood=true
  []
  [linearx]
    type = Linear
    # noise = 'mcmc_reporter/noise'
    noise = 'noise_specified/noise_specified'
    file_name = 'exp_lnx_noise_reduced.csv'
    # log_likelihood=false
  []
  [lineary]
    type = Linear
    # noise = 'mcmc_reporter/noise'
    noise = 'noise_specified/noise_specified'
    file_name = 'exp_lny_noise_reduced.csv'
    # log_likelihood=false
  []
[]

[Samplers]
  [sample]
    type = AffineInvariantDES
    prior_distributions = 'a b c d'
    num_parallel_proposals = 5
    file_name = 'confg_predprey_reduced.csv'
    execute_on =  PRE_MULTIAPP_SETUP
    seed = 100
    initial_values = '0.8 0.5 0.5 0.25'
    previous_state = 'mcmc_reporter/inputs'
    previous_state_var = 'mcmc_reporter/variance'
    # prior_variance = 'prior_variance'
    lower_bound = '0.0 0.0 0.0 0.0'
    upper_bound = '2.0 2.0 2.0 2.0'
    # variance_bound = 0.5
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = predprey_exact.i
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

[Reporters]
  [constant_x]
    type = StochasticReporter
  []
  [constant_y]
    type = StochasticReporter
  []
  [noise_specified]
    type = ConstantReporter
    real_names = 'noise_specified'
    real_values = '0.25'
  []
  [mcmc_reporter]
    type = AffineInvariantDifferentialDecisionwithGPry
    output_value = constant_x/reporter_transfer_x:log_x:value
    output_value1 = constant_y/reporter_transfer_y:log_y:value
    sampler = sample
    likelihoods = 'gaussianx gaussiany linearx lineary'
    gp_evaluator = GP_eval
  []
[]

[Surrogates]
  [GP_eval]
    type = GaussianProcess
    filename = 'gpry_train_predprey_out2_EIGF_new_GP_al_trainer.rd'
  []
[]

[Executioner]
  type = Transient
  num_steps = 500
[]

[Outputs]
  execute_on = TIMESTEP_END
  file_base = 'des_gpry_predprey'
  perf_graph = true
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
