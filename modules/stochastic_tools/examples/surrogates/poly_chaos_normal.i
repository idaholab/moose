[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Normal
    mean = 5
    standard_deviation = 2
  []
  [q_dist]
    type = Normal
    mean = 10000
    standard_deviation = 500
  []
  [L_dist]
    type = Normal
    mean = 0.03
    standard_deviation = 0.01
  []
  [Tinf_dist]
    type = Normal
    mean = 300
    standard_deviation = 10
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
