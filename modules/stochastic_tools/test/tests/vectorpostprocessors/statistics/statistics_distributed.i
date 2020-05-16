[StochasticTools]
[]

[VectorPostprocessors]
  [test]
    type = TestDistributedVectorPostprocessor
    parallel_type = distributed
    outputs = none
  []
  [stats]
    type = Statistics
    vectorpostprocessors = 'test'
    compute = 'min max sum mean stddev norm2 ratio'
  []
[]

[Outputs]
  execute_on = TIMESTEP_END
  csv = true
[]
