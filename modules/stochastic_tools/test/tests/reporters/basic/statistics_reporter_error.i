[StochasticTools]
[]

[Reporters]
  [const]
    type = ConstantReporter
    string_vector_names = 'const'
    string_vector_values = 'a b c d'
  []
  [stats]
    type = StatisticsReporter
    reporters = 'const/const'
    compute = 'min max sum mean stddev norm2 ratio stderr'
  []
[]

[Outputs/out]
  type = JSON
  execute_on = FINAL
[]
