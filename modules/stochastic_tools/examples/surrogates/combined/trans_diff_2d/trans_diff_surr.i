[StochasticTools]
[]

[Distributions]
  [C_dist]
    type = Uniform
    lower_bound = 0.01
    upper_bound = 0.02
  []
  [f_dist]
    type = Uniform
    lower_bound = 15
    upper_bound = 25
  []
  [init_dist]
    type = Uniform
    lower_bound = 270
    upper_bound = 330
  []
[]

[Samplers]
  [sample]
    type = LatinHypercube
    num_rows = 100000
    distributions = 'C_dist f_dist init_dist'
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[Surrogates]
  [pc_min]
    type = PolynomialChaos
    filename = 'trans_diff_trainer_out_pc_min.rd'
  []
  [pc_max]
    type = PolynomialChaos
    filename = 'trans_diff_trainer_out_pc_max.rd'
  []
  [pr_min]
    type = PolynomialRegressionSurrogate
    filename = 'trans_diff_trainer_out_pr_min.rd'
  []
  [pr_max]
    type = PolynomialRegressionSurrogate
    filename = 'trans_diff_trainer_out_pr_max.rd'
  []
  [np_min]
    type = NearestPointSurrogate
    filename = 'trans_diff_trainer_out_np_min.rd'
  []
  [np_max]
    type = NearestPointSurrogate
    filename = 'trans_diff_trainer_out_np_max.rd'
  []
[]

# Computing statistics
[VectorPostprocessors]
  [eval_surr]
    type = EvaluateSurrogate
    model = 'pc_max pc_min pr_max pr_min np_max np_min'
    sampler = sample
  []
  [eval_surr_stats]
    type = Statistics
    vectorpostprocessors = eval_surr
    compute = 'mean stddev'
    ci_method = 'percentile'
    ci_levels = '0.05'
  []
[]

[Outputs]
  csv = true
[]
