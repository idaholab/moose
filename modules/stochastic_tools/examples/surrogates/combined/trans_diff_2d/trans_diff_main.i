[StochasticTools]
[]

[Distributions]
  [C_dist]
    type = Uniform
    lower_bound = 0.01
    upper_bound = 0.02
  []
  [f_dist]
    type = Uniform
    lower_bound = 15
    upper_bound = 25
  []
  [init_dist]
    type = Uniform
    lower_bound = 270
    upper_bound = 330
  []
[]

[Samplers]
  [hypercube]
    type = LatinHypercube
    num_rows = 2000
    distributions = 'C_dist f_dist init_dist'
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = hypercube
    input_files = 'trans_diff_sub.i'
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = runner
    sampler = hypercube
    param_names = 'Materials/diff_coeff/constant_expressions Functions/src_func/vals Variables/T/initial_condition'
  []
[]

[Transfers]
  [results]
    type = SamplerPostprocessorTransfer
    from_multi_app = runner
    sampler = hypercube
    to_vector_postprocessor = results
    from_postprocessor = 'time_max time_min'
  []
[]

[VectorPostprocessors]
  [results]
    type = StochasticResults
  []
[]

[Reporters]
  [stats]
    type = StatisticsReporter
    vectorpostprocessors = results
    compute = 'mean stddev'
    ci_method = 'percentile'
    ci_levels = '0.05'
  []
[]

[Outputs]
  csv = true
[]
