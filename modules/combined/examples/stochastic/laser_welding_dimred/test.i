[StochasticTools]
[]

[Distributions]
  [R_dist]
    type = Uniform
    lower_bound = 1.25E-4
    upper_bound = 1.55E-4
  []
  [power_dist]
    type = Uniform
    lower_bound = 60
    upper_bound = 70
  []
[]

[Samplers]
  [test]
    type = MonteCarlo
    num_rows = 90
    distributions = 'R_dist power_dist'
    execute_on = PRE_MULTIAPP_SETUP
    min_procs_per_row = 2
    max_procs_per_row = 2
    seed=42
  []
[]

[MultiApps]
  [worker]
    type = SamplerFullSolveMultiApp
    input_files = 2d-reconst.i
    sampler = test
    mode = batch-reset
    min_procs_per_app = 2
    max_procs_per_app = 2
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = worker
    sampler = test
    param_names = 'R power'
  []
[]

[Transfers]
  [results]
    type = SamplerReporterTransfer
    from_multi_app = worker
    sampler = test
    stochastic_reporter = matrix
    from_reporter = 'l2error/value'
  []
[]

[Reporters]
  [matrix]
    type = StochasticMatrix
    sampler = test
    parallel_type = ROOT
  []
[]

[Outputs]
  [json]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = NONE
  []
[]
