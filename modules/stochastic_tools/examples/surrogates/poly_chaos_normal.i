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

[Reporters]
  [samp]
    type = EvaluateSurrogate
    model = 'poly_chaos_avg poly_chaos_max'
    sampler = sample
    parallel_type = ROOT
  []
  [stats]
    type = PolynomialChaosReporter
    pc_name = 'poly_chaos_avg poly_chaos_max'
    statistics = 'mean stddev'
    local_sensitivity_points = '5 10000 0.03 300; 5 10000 0.03 300'
    include_sobol = true
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = final
  []
[]
