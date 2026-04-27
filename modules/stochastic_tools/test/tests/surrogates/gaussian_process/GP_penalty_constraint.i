# Gaussian Process with penalty constraints during hyperparameter tuning.
# Penalty terms are added to the NLML loss to steer the Adam optimizer toward
# hyperparameter configurations that respect lower/upper bound constraints at
# specified evaluation points.

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
[]

[Samplers]
  [train_sample]
    type = MonteCarlo
    num_rows = 10
    distributions = 'k_dist q_dist'
    execute_on = PRE_MULTIAPP_SETUP
  []
  [test_sample]
    type = MonteCarlo
    num_rows = 20
    distributions = 'k_dist q_dist'
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = train_sample
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = train_sample
    param_names = 'Materials/conductivity/prop_values Kernels/source/value'
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    sampler = train_sample
    stochastic_reporter = results
    from_reporter = 'avg/value'
  []
[]

[Reporters]
  [results]
    type = StochasticReporter
    parallel_type = ROOT
  []
  [samp_avg]
    type = EvaluateSurrogate
    model = GP_avg
    sampler = test_sample
    evaluate_std = 'true'
    parallel_type = ROOT
    execute_on = final
  []
[]

[Trainers]
  [GP_avg_trainer]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    covariance_function = 'covar'
    standardize_params = 'true'
    standardize_data = 'true'
    sampler = train_sample
    response = results/data:avg:value
    tune_parameters = 'signal_variance noise_variance length_factor'
    num_iters = 50
    learning_rate = 0.001
    # Penalize predictions below 300 (floor near BC value) at two constraint points
    penalty_constraint_points = '5.5 9000
                                  5.5 11000'
    penalty_constraint_lower_bounds = '300.0 300.0'
    penalty_weight = 10.0
  []
[]

[Surrogates]
  [GP_avg]
    type = GaussianProcessSurrogate
    trainer = GP_avg_trainer
  []
[]

[Covariance]
  [covar]
    type = SquaredExponentialCovariance
    signal_variance = 1
    noise_variance = 1e-3
    length_factor = '0.4 0.4'
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
