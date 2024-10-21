x = 0
y = 0
z = 0

f1 = ${fparse 1*x+2*y+3*z}
f2 = ${fparse 4*x+5*y+6*z}
f3 = ${fparse 7*x+8*y+9*z}

[StochasticTools]
[]

[Reporters/const]
  type = ConstantReporter
  real_names = 'f1 f2 f3'
  real_values = '${f1} ${f2} ${f3}'
  real_vector_names = 'f_combined'
  real_vector_values = '${f1} ${f2} ${f3}'
[]
