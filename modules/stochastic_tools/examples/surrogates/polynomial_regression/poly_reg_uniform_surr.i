[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Uniform
    lower_bound = 1
    upper_bound = 10
  []
  [q_dist]
    type = Uniform
    lower_bound = 9000
    upper_bound = 11000
  []
  [L_dist]
    type = Uniform
    lower_bound = 0.01
    upper_bound = 0.05
  []
  [Tinf_dist]
    type = Uniform
    lower_bound = 290
    upper_bound = 310
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 100000
    distributions = 'k_dist q_dist L_dist Tinf_dist'
    execute_on = initial
  []
[]

[Surrogates]
  [poly_reg_avg]
    type = PolynomialRegression
    filename = 'poly_reg_training_poly_reg_avg.rd'
  []
  [poly_reg_max]
    type = PolynomialRegression
    filename = 'poly_reg_training_poly_reg_max.rd'
  []
[]

# Computing statistics
[VectorPostprocessors]
  [samp_avg]
    type = SurrogateTester
    model = poly_reg_avg
    sampler = sample
  []
  [samp_max]
    type = SurrogateTester
    model = poly_reg_max
    sampler = sample
  []
[]

# Computing statistics
[VectorPostprocessors]
  [stats_avg]
    type = Statistics
    vectorpostprocessors = 'samp_avg'
    compute = 'mean stddev'
  []
  [stats_max]
    type = Statistics
    vectorpostprocessors = 'samp_max'
    compute = 'mean stddev'
  []
[]

[Outputs]
  csv = true
[]
