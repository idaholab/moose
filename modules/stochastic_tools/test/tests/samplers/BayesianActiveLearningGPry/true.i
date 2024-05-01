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
    noise = 'mcmc_reporter/noise'
    file_name = 'exp_0_05.csv'
    log_likelihood = true
  []
[]

[Samplers]
  [sample]
    type = AffineInvariantDES
    prior_distributions = 'left right'
    num_parallel_proposals = 50
    file_name = 'confg.csv'
    execute_on = PRE_MULTIAPP_SETUP
    seed = 2547
    initial_values = '0.1 0.1'
    previous_state = 'mcmc_reporter/inputs'
    previous_state_var = 'mcmc_reporter/variance'
    prior_variance = 'variance'
    lower_bound = '-1.0 -1.0'
    upper_bound = '1.0 1.0'
    variance_bound = 0.1
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
  [mcmc_reporter]
    type = AffineInvariantDifferentialDecision
    output_value = constant/reporter_transfer:average:value
    sampler = sample
    likelihoods = 'gaussian'
  []
[]

[Executioner]
  type = Transient
  num_steps = 1000
[]

[Outputs]
  file_base = 'true'
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
