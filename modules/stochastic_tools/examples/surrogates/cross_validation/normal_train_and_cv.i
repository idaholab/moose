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
  [pr_sampler]
    type = LatinHypercube
    distributions = 'k_dist q_dist L_dist Tinf_dist'
    num_rows = 6560
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [pr_sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = pr_sampler
    mode = batch-reset
  []
[]

[Controls]
  [pr_cmdline]
    type = MultiAppSamplerControl
    multi_app = pr_sub
    sampler = pr_sampler
    param_names = 'Materials/conductivity/prop_values Kernels/source/value Mesh/xmax BCs/right/value'
  []
[]

[Transfers]
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
    outputs = none
  []
  [cv_scores]
    type = CrossValidationScores
    models = 'pr_surr'
    execute_on = FINAL
  []
[]

[Trainers]
  [pr_max]
    type = PolynomialRegressionTrainer
    regression_type = "ols"
    execute_on = timestep_end
    max_degree = 3
    sampler = pr_sampler
    response = results/pr_data:max:value
    cv_type = "k_fold"
    cv_splits = 5
    cv_surrogate = "pr_surr"
    cv_n_trials = 100
  []
[]

[Surrogates]
  [pr_surr]
    type = PolynomialRegressionSurrogate
    trainer = pr_max
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
