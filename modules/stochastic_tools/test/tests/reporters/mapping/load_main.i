[StochasticTools]
[]

[Distributions]
  [S_dist]
    type = Uniform
    lower_bound = 2.5
    upper_bound = 7.5
  []
  [D_dist]
    type = Uniform
    lower_bound = 2.5
    upper_bound = 7.5
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 8
    distributions = 'S_dist D_dist'
    execute_on = initial
    min_procs_per_row = 2
  []
[]

[MultiApps]
  [worker]
    type = SamplerFullSolveMultiApp
    input_files = map_sub.i
    sampler = sample
    mode = batch-restore
    min_procs_per_app = 2
  []
[]

[Transfers]
  [param_transfer]
    type = SamplerParameterTransfer
    to_multi_app = worker
    sampler = sample
    parameters = 'Kernels/source_u/value BCs/right_v/value'
  []
  [data]
    type = SamplerReporterTransfer
    from_multi_app = worker
    stochastic_reporter = results
    from_reporter = 'pod_coeffs/u_pod pod_coeffs/v_pod'
    sampler = sample
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
  []
[]

[Outputs]
  [json]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = none
  []
  file_base = map_variable
[]
