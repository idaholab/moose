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
  [poly_chaos_avg]
    type = PolynomialChaos
    filename = 'poly_chaos_training_poly_chaos_avg.rd'
  []
  [poly_chaos_max]
    type = PolynomialChaos
    filename = 'poly_chaos_training_poly_chaos_max.rd'
  []
[]

# Computing statistics
[VectorPostprocessors]
  [samp_avg]
    type = EvaluateSurrogate
    model = poly_chaos_avg
    sampler = sample
  []
  [samp_max]
    type = EvaluateSurrogate
    model = poly_chaos_max
    sampler = sample
  []
  [stats_avg]
    type = PolynomialChaosStatistics
    pc_name = 'poly_chaos_avg'
    compute = 'mean stddev'
  []
  [stats_max]
    type = PolynomialChaosStatistics
    pc_name = 'poly_chaos_max'
    compute = 'mean stddev'
  []
  [sense_avg]
    type = PolynomialChaosLocalSensitivity
    pc_name = 'poly_chaos_avg'
    local_points = '5 10000 0.03 300'
  []
  [sense_max]
    type = PolynomialChaosLocalSensitivity
    pc_name = 'poly_chaos_max'
    local_points = '5 10000 0.03 300'
  []
  [sobol_avg]
    type = PolynomialChaosSobolStatistics
    pc_name = 'poly_chaos_avg'
    sensitivity_order = 'first second'
  []
  [sobol_max]
    type = PolynomialChaosSobolStatistics
    pc_name = 'poly_chaos_max'
    sensitivity_order = 'first second'
  []
[]

[Outputs]
  csv = true
[]
