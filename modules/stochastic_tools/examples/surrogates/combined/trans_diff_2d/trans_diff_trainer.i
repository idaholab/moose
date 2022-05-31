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
    type = Quadrature
    order = 5
    distributions = 'C_dist f_dist init_dist'
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    input_files = 'trans_diff_sub.i'
    sampler = sample
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = runner
    sampler = sample
    param_names = 'Materials/diff_coeff/constant_expressions Functions/src_func/vals Variables/T/initial_condition'
  []
[]

[Transfers]
  [results]
    type = SamplerReporterTransfer
    from_multi_app = runner
    sampler = sample
    stochastic_reporter = trainer_results
    from_reporter = 'time_max/value time_min/value'
  []
[]

[Reporters]
  [trainer_results]
    type = StochasticReporter
  []
[]

[Trainers]
  [pc_max]
    type = PolynomialChaosTrainer
    execute_on = final
    order = 5
    distributions = 'C_dist f_dist init_dist'
    sampler = sample
    response = trainer_results/results:time_max:value
  []
  [pc_min]
    type = PolynomialChaosTrainer
    execute_on = final
    order = 5
    distributions = 'C_dist f_dist init_dist'
    sampler = sample
    response = trainer_results/results:time_min:value
  []
  [np_max]
    type = NearestPointTrainer
    execute_on = final
    sampler = sample
    response = trainer_results/results:time_max:value
  []
  [np_min]
    type = NearestPointTrainer
    execute_on = final
    sampler = sample
    response = trainer_results/results:time_min:value
  []
  [pr_max]
    type = PolynomialRegressionTrainer
    regression_type = "ols"
    execute_on = final
    max_degree = 4
    sampler = sample
    response = trainer_results/results:time_max:value
  []
  [pr_min]
    type = PolynomialRegressionTrainer
    regression_type = "ols"
    execute_on = final
    max_degree = 4
    sampler = sample
    response = trainer_results/results:time_min:value
  []
[]

[Outputs]
  [out]
    type = SurrogateTrainerOutput
    trainers = 'pc_max pc_min np_max np_min pr_max pr_min'
    execute_on = FINAL
  []
[]
