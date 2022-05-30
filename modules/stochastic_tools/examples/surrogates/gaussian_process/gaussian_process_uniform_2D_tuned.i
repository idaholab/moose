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
    num_rows = 50
    distributions = 'k_dist q_dist'
    execute_on = PRE_MULTIAPP_SETUP
  []
  [cart_sample]
    type = CartesianProduct
    linear_space_items = '1 0.09 100
                          9000 20 100 '
    execute_on = initial
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
  []
[]


[Trainers]
  [GP_avg_trainer]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    covariance_function = 'rbf'
    standardize_params = 'true'               #Center and scale the training params
    standardize_data = 'true'                 #Center and scale the training data
    tao_options = '-tao_bncg_type kd'
    sampler = train_sample
    response = results/data:avg:value
    tune_parameters = ' signal_variance length_factor'
    tuning_min = ' 1e-9 1e-9'
    tuning_max = ' 1e16  1e16'
    tuning_algorithm = 'tao'
  []
[]


[Covariance]
  [rbf]
    type=SquaredExponentialCovariance
    signal_variance = 1                       #Use a signal variance of 1 in the kernel
    noise_variance = 1e-3                     #A small amount of noise can help with numerical stability
    length_factor = '0.38971 0.38971'         #Select a length factor for each parameter (k and q)
  []
[]

[Surrogates]
  [GP_avg]
    type = GaussianProcess
    trainer = 'GP_avg_trainer'
  []
[]

[Reporters]
  [train_avg]
    type = EvaluateSurrogate
    model = GP_avg
    sampler = train_sample
    evaluate_std = 'true'
    parallel_type = ROOT
    execute_on = final
  []
  [cart_avg]
    type = EvaluateSurrogate
    model = GP_avg
    sampler = cart_sample
    evaluate_std = 'true'
    parallel_type = ROOT
    execute_on = final
  []
[]

[VectorPostprocessors]
  [hyperparams]
    type = GaussianProcessData
    gp_name = 'GP_avg'
    execute_on = final
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
