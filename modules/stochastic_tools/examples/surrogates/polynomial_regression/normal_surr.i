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
    num_bins = 20
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

# Computing statistics
[VectorPostprocessors]
  [pc_max_res]
    type = EvaluateSurrogate
    model = pc_max
    sampler = sample
  []
  [pr_max_res]
    type = EvaluateSurrogate
    model = pr_max
    sampler = sample
  []
  [pc_max_stats]
    type = PolynomialChaosStatistics
    pc_name = 'pc_max'
    compute = 'mean stddev'
  []
  [pr_max_stats]
    type = Statistics
    vectorpostprocessors = pr_max_res
    compute = 'mean stddev'
  []
[]

[Outputs]
  csv = true
[]
