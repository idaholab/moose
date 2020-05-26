[StochasticTools]
[]

[Distributions]
  [mu1_dist]
    type = Normal
    mean = 0.3
    standard_deviation = 0.045
  []
  [mu2_dist]
    type = Normal
    mean = 9
    standard_deviation = 1.35
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 200000
    distributions = 'mu1_dist mu2_dist'
    execute_on = initial
  []
[]

[Surrogates]
  [poly_chaos_max]
    type = PolynomialChaos
    filename = 'poly_chaos_training_poly_chaos_max.rd'
  []
  [poly_chaos_min]
    type = PolynomialChaos
    filename = 'poly_chaos_training_poly_chaos_min.rd'
  []
  [poly_chaos_avg]
    type = PolynomialChaos
    filename = 'poly_chaos_training_poly_chaos_avg.rd'
  []
[]

# Computing statistics
[VectorPostprocessors]
  [samp_max]
    type = SurrogateTester
    model = poly_chaos_max
    sampler = sample
  []
  [samp_min]
    type = SurrogateTester
    model = poly_chaos_min
    sampler = sample
  []
  [samp_avg]
    type = SurrogateTester
    model = poly_chaos_avg
    sampler = sample
  []
  [stats_max]
    type = PolynomialChaosStatistics
    pc_name = 'poly_chaos_max'
    compute = 'mean stddev'
  []
  [stats_min]
    type = PolynomialChaosStatistics
    pc_name = 'poly_chaos_min'
    compute = 'mean stddev'
  []
  [stats_avg]
    type = PolynomialChaosStatistics
    pc_name = 'poly_chaos_avg'
    compute = 'mean stddev'
  []
  [sobol_max]
    type = PolynomialChaosSobolStatistics
    pc_name = 'poly_chaos_max'
    sensitivity_order = 'first second'
  []
  [sobol_min]
    type = PolynomialChaosSobolStatistics
    pc_name = 'poly_chaos_min'
    sensitivity_order = 'first second'
  []
  [sobol_avg]
    type = PolynomialChaosSobolStatistics
    pc_name = 'poly_chaos_avg'
    sensitivity_order = 'first second'
  []
[]

[Outputs]
  csv = true
[]
