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
    type = LatinHypercube
    num_rows = 100000
    distributions = 'k_dist q_dist L_dist Tinf_dist'
    num_bins = 20
  []
[]

[Surrogates]
  [pc_max]
    type = PolynomialChaos
    filename = 'uniform_train_pc_out_pc_max.rd'
  []
  [pr_max]
    type = PolynomialRegressionSurrogate
    filename = 'uniform_train_pr_out_pr_max.rd'
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
