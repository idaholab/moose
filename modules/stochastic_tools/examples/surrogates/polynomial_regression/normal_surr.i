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
    type = LatinHypercube
    num_rows = 100000
    distributions = 'k_dist q_dist L_dist Tinf_dist'
  []
[]

[Surrogates]
  [pc_max]
    type = PolynomialChaos
    filename = 'normal_train_pc_out_pc_max.rd'
  []
  [pr_max]
    type = PolynomialRegressionSurrogate
    filename = 'normal_train_pr_out_pr_max.rd'
  []
[]

[Reporters]
  [pc_max_res]
    type = EvaluateSurrogate
    model = pc_max
    sampler = sample
    parallel_type = ROOT
  []
  [pr_max_res]
    type = EvaluateSurrogate
    model = pr_max
    sampler = sample
    parallel_type = ROOT
  []
  [pr_max_stats]
    type = StatisticsReporter
    reporters = 'pr_max_res/pr_max'
    compute = 'mean stddev'
  []
  [pc_max_stats]
    type = PolynomialChaosReporter
    pc_name = 'pc_max'
    statistics = 'mean stddev'
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = timestep_end
  []
[]
