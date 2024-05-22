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
    file_name = 'exp_lnx_noise.csv'
    log_likelihood=true
  []
  [gaussiany]
    type = Gaussian
    noise = 'mcmc_reporter/noise'
    file_name = 'exp_lny_noise.csv'
    log_likelihood=true
  []
[]

[Samplers]
  [sample]
    type = AffineInvariantDES
    prior_distributions = 'a b c d'
    previous_state = 'mcmc_reporter/inputs'
    num_parallel_proposals = 5
    lower_bound = '0.0 0.0 0.0 0.0'
    upper_bound = '2.0 2.0 2.0 2.0'
    num_columns = 1
    file_name = 'confg_predprey.csv'
    previous_state_var = 'mcmc_reporter/variance'
    prior_variance = 'prior_variance'
    execute_on = PRE_MULTIAPP_SETUP
    seed = 2547
    initial_values = '0.5 0.5 0.5 0.5'
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
    type = AffineInvariantDifferentialDecision
    output_value = constant_x/reporter_transfer_x:log_x:value
    output_value1 = constant_y/reporter_transfer_y:log_y:value
    sampler = sample
    likelihoods = 'gaussianx gaussiany'
  []
[]

[Executioner]
  type = Transient
  num_steps = 500
[]

[Outputs]
  file_base = 'des_10prop_noise'
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
