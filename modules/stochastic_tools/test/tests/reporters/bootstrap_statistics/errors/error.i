[StochasticTools]
[]

[VectorPostprocessors]
  [treatment]
    type = ConstantVectorPostprocessor
    value = '94 197 16 38 99 141 23'
    outputs = none
  []
  [control]
    type = ConstantVectorPostprocessor
    value = '52 104 146 10 51 30 40 27 46'
    outputs = none
  []
[]

[Reporters]
  [stats]
    type = StatisticsReporter
    compute = 'mean stderr'
    ci_method = 'percentile'
    ci_levels = '0.025 0.05 0.1 0.16 0.5'
    ci_replicates = 1000
    ci_seed = 1980
  []
[]
