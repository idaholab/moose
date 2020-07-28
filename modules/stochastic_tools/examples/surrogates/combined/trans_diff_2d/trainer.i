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
    type = MultiAppCommandLineControl
    multi_app = runner
    sampler = sample
    param_names = 'Materials/diff_coeff/constant_expressions Functions/src_func/vals Variables/u/initial_condition'
  []
[]

[Transfers]
  [results]
    type = SamplerPostprocessorTransfer
    multi_app = runner
    sampler = sample
    to_vector_postprocessor = trainer_results
    from_postprocessor = 'time_max time_min'
  []
[]

[VectorPostprocessors]
  [trainer_results]
    type = StochasticResults
  []
[]

[Trainers]
  [pc_max]
    type = PolynomialChaosTrainer
    execute_on = final
    order = 5
    distributions = 'C_dist f_dist init_dist'
    sampler = sample
    results_vpp = trainer_results
    results_vector = results:time_max
  []
  [pc_min]
    type = PolynomialChaosTrainer
    execute_on = final
    order = 5
    distributions = 'C_dist f_dist init_dist'
    sampler = sample
    results_vpp = trainer_results
    results_vector = results:time_min
  []
  [np_max]
    type = NearestPointTrainer
    execute_on = final
    sampler = sample
    results_vpp = trainer_results
    results_vector = results:time_max
  []
  [np_min]
    type = NearestPointTrainer
    execute_on = final
    sampler = sample
    results_vpp = trainer_results
    results_vector = results:time_min
  []
  [pr_max]
    type = PolynomialRegressionTrainer
    regression_type = "ols"
    execute_on = final
    max_degree = 4
    sampler = sample
    results_vpp = trainer_results
    results_vector = results:time_max
  []
  [pr_min]
    type = PolynomialRegressionTrainer
    regression_type = "ols"
    execute_on = final
    max_degree = 4
    sampler = sample
    results_vpp = trainer_results
    results_vector = results:time_min
  []
[]

[Outputs]
  [out]
    type = SurrogateTrainerOutput
    trainers = 'pc_max pc_min np_max np_min pr_max pr_min'
    execute_on = FINAL
  []
  [pgraph]
    type = PerfGraphOutput
    execute_on = 'initial final'  # Default is "final"
    level = 4                    # Default is 1
    heaviest_branch = true        # Default is false
    heaviest_sections = 7         # Default is 0
  []
[]
