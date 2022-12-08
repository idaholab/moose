[StochasticTools]
[]

[VectorPostprocessors]
  [data]
    type = TestDistributedVectorPostprocessor
    parallel_type = distributed
    outputs = none
  []
[]

[Reporters]
  [stats]
    type = StatisticsReporter
    vectorpostprocessors = 'data'
    compute = 'mean'
    ci_method = 'bca'
    ci_levels = '0.025 0.05 0.1 0.16 0.5 0.84 0.9 0.95 0.99'
    ci_replicates = 10
  []
[]

[Outputs]
  execute_on = FINAL
  [out]
    type = JSON
  []
[]
