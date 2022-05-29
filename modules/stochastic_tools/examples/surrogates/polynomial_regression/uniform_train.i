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
  [pc_sampler]
    type = Quadrature
    order = 8
    distributions = 'k_dist q_dist L_dist Tinf_dist'
    execute_on = PRE_MULTIAPP_SETUP
  []
  [pr_sampler]
    type = LatinHypercube
    distributions = 'k_dist q_dist L_dist Tinf_dist'
    num_rows = 6560
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [pc_sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = pc_sampler
  []
  [pr_sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = pr_sampler
  []
[]

[Controls]
  [pc_cmdline]
    type = MultiAppSamplerControl
    multi_app = pc_sub
    sampler = pc_sampler
    param_names = 'Materials/conductivity/prop_values Kernels/source/value Mesh/xmax BCs/right/value'
  []
  [pr_cmdline]
    type = MultiAppSamplerControl
    multi_app = pr_sub
    sampler = pr_sampler
    param_names = 'Materials/conductivity/prop_values Kernels/source/value Mesh/xmax BCs/right/value'
  []
[]

[Transfers]
  [pc_data]
    type = SamplerReporterTransfer
    from_multi_app = pc_sub
    sampler = pc_sampler
    stochastic_reporter = results
    from_reporter = 'max/value'
  []
  [pr_data]
    type = SamplerReporterTransfer
    from_multi_app = pr_sub
    sampler = pr_sampler
    stochastic_reporter = results
    from_reporter = 'max/value'
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
  []
[]

[Trainers]
  [pc_max]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 8
    distributions = 'k_dist q_dist L_dist Tinf_dist'
    sampler = pc_sampler
    response = results/pc_data:max:value
  []
  [pr_max]
    type = PolynomialRegressionTrainer
    regression_type = "ols"
    execute_on = timestep_end
    max_degree = 4
    sampler = pr_sampler
    response = results/pr_data:max:value
  []
[]

[Outputs]
  [pc_out]
    type = SurrogateTrainerOutput
    trainers = 'pc_max'
    execute_on = FINAL
  []
  [pr_out]
    type = SurrogateTrainerOutput
    trainers = 'pr_max'
    execute_on = FINAL
  []
[]
