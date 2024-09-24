[StochasticTools]
[]

[Distributions]
  [R_dist]
    type = Uniform
    lower_bound = 1.25e-4
    upper_bound = 1.7e-4
  []
  [power_dist]
    type = Uniform
    lower_bound = 70
    upper_bound = 83
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    seed = 13
    num_rows = 1
    distributions = 'R_dist power_dist'
    min_procs_per_row = 56
    max_procs_per_row = 56
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [worker]
    type = SamplerFullSolveMultiApp
    input_files = 2d-error.i
    sampler = sample
    mode = batch-reset
    min_procs_per_app = 56
    max_procs_per_app = 56
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = worker
    sampler = sample
    param_names = 'R power'
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = worker
    sampler = sample
    from_reporter = 'relativel2error/value'
    stochastic_reporter = 'matrix'
  []
[]

[Reporters]
  [matrix]
    type = StochasticMatrix
    sampler = sample
    parallel_type = ROOT
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = NONE
  []
[]
