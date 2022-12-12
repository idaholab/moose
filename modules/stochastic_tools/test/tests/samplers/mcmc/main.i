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
    proposal_std = 'mcmc_reporter/proposal_std' # This is just a placeholder for now
    num_parallel_proposals = 10
    std_prop = '0.2 0.2'
    # lb = '-3.0 -3.0' # if needed
    # ub = '3.0 3.0' # if needed
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
    type = PMCMCDecision
    seed_inputs = 'seed_inputs'
    output_value = constant/reporter_transfer:average:value
    inputs = 'inputs'
    sampler = sample
    likelihoods = 'gaussian'
    prior_distributions = 'left right'
  []
[]

[Executioner]
  type = Transient
  num_steps = 500
[]

[Outputs]
  file_base = 'test_mcmc'
  [out]
    type = JSON
    execute_system_information_on = NONE
  []
[]
