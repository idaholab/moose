[StochasticTools]
[]

[Reporters]
  [const]
    type = ConstantReporter
    real_vector_names = 'real'
    real_vector_values = '2 4 6 8 10'

    integer_vector_names = 'int'
    integer_vector_values = '100 200 300 400 500'
  []
  [stats]
    type = StatisticsReporter
    reporters = 'const/real const/int'
    compute = 'min max sum mean stddev norm2 ratio stderr'
  []
[]

[Outputs/out]
  type = JSON
  execute_on = FINAL
[]
