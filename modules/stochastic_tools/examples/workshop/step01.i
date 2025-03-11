[StochasticTools]
[]

[Distributions]
  [D]
    type = Uniform
    lower_bound = 0.5
    upper_bound = 2.5
  []
  [q]
    type = Normal
    mean = 100
    standard_deviation = 25
  []
  [T_0]
    type = Normal
    mean = 300
    standard_deviation = 45
  []
  [q_0]
    type = Weibull
    location = -110
    scale = 20
    shape = 1
  []
[]

[Samplers]
  [hypercube]
    type = LatinHypercube
    num_rows = 5000
    distributions = 'D q T_0 q_0'
  []
[]

[Reporters]
  [sampling_matrix]
    type = StochasticMatrix
    sampler = hypercube
    sampler_column_names = 'D q T_0 q_0'
    parallel_type = ROOT
  []
[]

[Outputs]
  csv = true
[]
