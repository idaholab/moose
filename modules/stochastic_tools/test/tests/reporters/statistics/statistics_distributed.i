[StochasticTools]
[]

[VectorPostprocessors]
  [test]
    type = TestDistributedVectorPostprocessor
    parallel_type = distributed
    outputs = none
  []
[]

[Reporters]
  [stats]
    type = StatisticsReporter
    vectorpostprocessors = 'test'
    compute = 'min max sum mean stddev norm2 ratio stderr'
  []
[]

[Outputs]
  execute_on = FINAL
  [out]
    type = JSON
  []
[]
