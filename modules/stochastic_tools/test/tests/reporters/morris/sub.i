q0 = 0
q1 = 0.5
q2 = 3
q3 = 9
q4 = 99
q5 = 99

x0 = 0
x1 = 0
x2 = 0
x3 = 0
x4 = 0
x5 = 0

y0 = ${fparse (abs(4 * x0 - 2) + q0) / (1 + q0)}
y1 = ${fparse (abs(4 * x1 - 2) + q1) / (1 + q1)}
y2 = ${fparse (abs(4 * x2 - 2) + q2) / (1 + q2)}
y3 = ${fparse (abs(4 * x3 - 2) + q3) / (1 + q3)}
y4 = ${fparse (abs(4 * x4 - 2) + q4) / (1 + q4)}
y5 = ${fparse (abs(4 * x5 - 2) + q5) / (1 + q5)}
y = ${fparse y0 * y1 * y2 * y3 * y4 * y5}

ya0 = ${fparse abs(4 * x0 - 2)}
ya1 = ${fparse abs(4 * x1 - 2)}
ya2 = ${fparse abs(4 * x2 - 2)}
ya3 = ${fparse abs(4 * x3 - 2)}
ya = ${fparse ya0 * ya1 * ya2 * ya3}

[StochasticTools]
[]

[Reporters/const]
  type = ConstantReporter
  real_names = 'gf gfa'
  real_values = '${y} ${ya}'
  real_vector_names = 'gf_vec'
  real_vector_values = '${y} ${ya}'
[]
