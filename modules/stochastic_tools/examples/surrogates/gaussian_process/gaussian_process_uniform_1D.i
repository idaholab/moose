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
  [train_sample]
    type = MonteCarlo
    num_rows = 6
    distributions = 'q_dist'
    execute_on = PRE_MULTIAPP_SETUP
  []
  [cart_sample]
    type = CartesianProduct
    linear_space_items = '9000 20 100'
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
    param_names = 'Kernels/source/value'
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
  [cart_avg]
    type = EvaluateSurrogate
    model = gauss_process_avg
    sampler = cart_sample
    evaluate_std = 'true'
    parallel_type = ROOT
    execute_on = final
  []
  [train_avg]
    type = EvaluateSurrogate
    model = gauss_process_avg
    sampler = train_sample
    evaluate_std = 'true'
    parallel_type = ROOT
    execute_on = final
  []
[]

[Trainers]
  [GP_avg_trainer]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    sampler = train_sample
    response = results/data:avg:value
    covariance_function = 'rbf'
    standardize_params = 'true'               #Center and scale the training params
    standardize_data = 'true'                 #Center and scale the training data
  []
[]

[Covariance]
  [rbf]
    type=SquaredExponentialCovariance
    signal_variance = 1                       #Use a signal variance of 1 in the kernel
    noise_variance = 1e-3                     #A small amount of noise can help with numerical stability
    length_factor = '0.38971'         #Select a length factor for each parameter (k and q)
  []
[]

[Surrogates]
  [gauss_process_avg]
    type = GaussianProcess
    trainer = 'GP_avg_trainer'
  []
[]

[Outputs]
  csv = true
  execute_on = FINAL
[]
