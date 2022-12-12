[StochasticTools]
[]

[Distributions]
  [mu1]
    type = Normal
    mean = 0.3
    standard_deviation = 0.045
  []
  [mu2]
    type = Normal
    mean = 9
    standard_deviation = 1.35
  []
[]

[Samplers]
  [hypercube]
    type = LatinHypercube
    num_rows = 5000
    distributions = 'mu1 mu2'
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = hypercube
    input_files = 'nonlin_diff_react_sub.i'
    mode = batch-restore
  []
[]

[Transfers]
  [parameters]
    type = SamplerParameterTransfer
    to_multi_app = runner
    sampler = hypercube
    parameters = 'Kernels/nonlin_function/mu1 Kernels/nonlin_function/mu2'
  []
  [results]
    type = SamplerPostprocessorTransfer
    from_multi_app = runner
    sampler = hypercube
    to_vector_postprocessor = results
    from_postprocessor = 'max min average'
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
    compute = 'mean'
    ci_method = 'percentile'
    ci_levels = '0.05'
  []
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
