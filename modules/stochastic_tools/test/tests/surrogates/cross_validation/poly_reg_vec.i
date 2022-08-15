[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Normal
    mean = 5
    standard_deviation = 2
  []
  [L_dist]
    type = Normal
    mean = 0.03
    standard_deviation = 0.01
  []
[]

[Samplers]
  [sample]
    type = LatinHypercube
    num_rows = 10
    distributions = 'k_dist L_dist'
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[GlobalParams]
  sampler = sample
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub_vector.i
    mode = batch-reset
    execute_on = initial
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    param_names = 'Materials/conductivity/prop_values L'
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    stochastic_reporter = results
    from_reporter = 'T_vec/T T_vec/x'
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
    outputs = none
  []
  [cv_scores]
    type = CrossValidationScores
    models = pr_surrogate
    execute_on = FINAL
  []
[]

[Trainers]
  [pr_trainer]
    type = PolynomialRegressionTrainer
    regression_type = "ols"
    sampler = sample
    response = results/data:T_vec:T
    response_type = vector_real
    execute_on = timestep_end
    max_degree = 1
    cv_type = "k_fold"
    cv_splits = 2
    cv_n_trials = 3
    cv_surrogate = pr_surrogate
    cv_seed = 1
  []
[]

[Surrogates]
  [pr_surrogate]
    type = PolynomialRegressionSurrogate
    trainer = pr_trainer
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
  []
[]
