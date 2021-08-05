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

[Reporters]
  # Sampling surrogate
  [samp]
    type = EvaluateSurrogate
    model = 'nearest_point_avg nearest_point_max'
    sampler = sample
    parallel_type = ROOT
  []
  # Computing statistics
  [stats]
    type = StatisticsReporter
    reporters = 'samp/nearest_point_avg samp/nearest_point_max'
    compute = 'mean stddev'
  []
[]

[Surrogates]
  [nearest_point_avg]
    type = NearestPointSurrogate
    filename = 'nearest_point_training_out_nearest_point_avg.rd'
  []
  [nearest_point_max]
    type = NearestPointSurrogate
    filename = 'nearest_point_training_out_nearest_point_max.rd'
  []
[]

[Outputs]
  csv = true
[]
