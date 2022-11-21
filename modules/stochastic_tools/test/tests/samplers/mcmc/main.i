[StochasticTools]
[]

[Distributions]
  [left]
    type = Normal
    mean = 0.0 # 0.15 #
    standard_deviation = 1.0 # 0.15 #
  []
  [right]
    type = Normal
    mean = 0.0 # -1.5 #
    standard_deviation = 1.0 # 0.15 # 
  []
[]

[Likelihoods]
  [gaussian]
    type = Gaussian
    noise = 0.05
    file_name = 'exp_0_05.csv'
    log_likelihood=true
  []
[]

[Samplers]
  [sample]
    type = PMCMCBase
    prior_distributions = 'left right'
    seed_inputs = 'mcmc_reporter/seed_inputs'
    num_parallel_proposals = 1
    initial_values = '0.05 0.05'
    file_name = 'confg.csv'
    execute_on = PRE_MULTIAPP_SETUP
    seed = 2547
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
  # [param]
  #   type = SamplerParameterTransfer
  #   to_multi_app = sub
  #   sampler = sample
  #   parameters = 'BCs/left/value BCs/right/value Mesh/xmax'
  #   to_control = 'stochastic'
  # []
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
    # execute_on = 'FINAL'
  []
  [mcmc_reporter]
    type = PMCMCDecision
    seed_inputs = 'seed_inputs'
    output_value = constant/reporter_transfer:average:value
    inputs = 'inputs'
    sampler = sample
    likelihoods = 'gaussian'
    prior_distributions = 'left right'
    # execute_on = 'FINAL'
  []
[]

[Executioner]
  type = Transient
  num_steps = 300
[]

[Outputs]
  file_base = 'test_mcmc'
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
