[StochasticTools]
[]

val = 1

[Reporters/const]
  type = ConstantReporter
  real_names = 'num'
  real_values = '${val}'
  integer_names = 'int'
  integer_values = '${val}'
  real_vector_names = 'vec'
  real_vector_values = '${val} ${val} ${val} ${val} ${val} ${val} ${val}'
[]
