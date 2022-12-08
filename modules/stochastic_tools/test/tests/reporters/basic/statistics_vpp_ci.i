[StochasticTools]
[]

[VectorPostprocessors/const]
  type = ConstantVectorPostprocessor
  value = '1 2 3 4 5'
[]

[Reporters/stats]
  type = StatisticsReporter
  vectorpostprocessors = 'const'
  compute = 'min max sum mean stddev norm2 ratio stderr median'
  ci_method = percentile
[]

[Outputs/out]
  type = JSON
  execute_on = FINAL
[]
