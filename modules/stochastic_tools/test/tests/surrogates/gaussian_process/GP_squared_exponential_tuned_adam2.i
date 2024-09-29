[StochasticTools]
[]
[Distributions]
  [a_dist]
    type = TruncatedNormal
    mean = -0.287682
    standard_deviation = 0.25
    lower_bound = -0.693147
    upper_bound = 0.405465
  []
  [b_dist]
    type = TruncatedNormal
    mean = -0.287682
    standard_deviation = 0.25
    lower_bound = -0.693147
    upper_bound = 0.405465
  []
  [c_dist]
    type = TruncatedNormal
    mean = -0.287682
    standard_deviation = 0.25
    lower_bound = -0.693147
    upper_bound = 0.405465
  []
  [d_dist]
    type = TruncatedNormal
    mean = -0.287682
    standard_deviation = 0.25
    lower_bound = -0.693147
    upper_bound = 0.405465
  []
  [e_dist]
    type = Uniform
    lower_bound = 0.5
    upper_bound = 1.5
  []
  [f_dist]
    type = TruncatedNormal
    mean = -3.688879
    standard_deviation = 0.5
    lower_bound = -5.2983
    upper_bound = -2.9957
  []
[]
[Samplers]
  [train_sample]
    type = MonteCarlo
    num_rows = 30
    distributions = 'a_dist b_dist c_dist d_dist e_dist f_dist'
    execute_on = PRE_MULTIAPP_SETUP
    seed = 100
  []
  [test_sample]
    type = MonteCarlo
    num_rows = 100
    distributions = 'a_dist b_dist c_dist d_dist e_dist f_dist'
    execute_on = PRE_MULTIAPP_SETUP
    seed = 101
  []
[]
[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = Navier_Stokes.i
    sampler = train_sample
  []
[]
[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = train_sample
    param_names = 'param1 param2 param3 param4 rho1 mu1'
  []
[]
[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    sampler = train_sample
    stochastic_reporter = results
    from_reporter = 'resultant_velocity/value'
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
    outputs = out
  []
  [train_avg]
    type = EvaluateSurrogate
    model = GP_avg
    sampler = train_sample
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
  [data]
    type = SamplerData
    sampler = test_sample
    execute_on = 'initial timestep_end'
  []
[]
[Trainers]
  [GP_avg_trainer]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    covariance_function = 'covar' #Choose a squared exponential for the kernel
    standardize_params = 'true' #Center and scale the training params
    standardize_data = 'true' #Center and scale the training data
    sampler = train_sample
    response = results/data:resultant_velocity:value
    tune_parameters = 'covar:signal_variance covar:length_factor'
    num_iters = 5000
    batch_size = 30
    learning_rate = 0.005
    show_every_nth_iteration = 1
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
    signal_variance = 1.0 #Use a signal variance of 1 in the kernel
    noise_variance = 1e-6 #A small amount of noise can help with numerical stability
    length_factor = '1.0 1.0 1.0 1.0 1.0 1.0' #Select a length factor for each parameter (k and q)
  []
[]
[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
    file_base = 'gp'
  []
[]