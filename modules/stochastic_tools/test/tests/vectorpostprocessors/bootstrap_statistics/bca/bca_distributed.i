[StochasticTools]
[]

[VectorPostprocessors]
  [data]
    type = TestDistributedVectorPostprocessor
    parallel_type = distributed
    outputs = none
  []

  [stats]
    type = Statistics
    vectorpostprocessors = 'data'
    compute = 'mean'
    ci_method = 'bca'
    ci_levels = '0.025 0.05 0.1 0.16 0.5'
    ci_replicates = 10
  []
[]

[Outputs]
  execute_on = TIMESTEP_END
  csv = true
[]
