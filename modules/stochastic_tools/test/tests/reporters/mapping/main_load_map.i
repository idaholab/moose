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
  [sample_test]
    type = MonteCarlo
    num_rows = 100
    distributions = 'S_dist D_dist'
    execute_on = initial
    min_procs_per_row = 2
    seed = 11
  []
[]

[MultiApps]
  [worker]
    type = SamplerFullSolveMultiApp
    input_files = sub_load_map.i
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
    from_reporter = 'pod_coeffs/v_pod'
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
  []
[]

[Outputs]
  [josn]
    type = JSON
    execute_on = FINAL
  []
[]
