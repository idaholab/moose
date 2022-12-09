[StochasticTools]
[]

[VectorPostprocessors]
  [const]
    type = ConstantVectorPostprocessor
    value = '1 2 3 4 5'
    outputs = none
  []
[]

[Reporters]
  [stats]
    type = StatisticsReporter
    vectorpostprocessors = 'const'
    compute = 'min max sum mean stddev norm2'
  []
[]

[Outputs]
  execute_on = FINAL
  [out]
    type = JSON
  []
[]
