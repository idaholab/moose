[StochasticTools]
[]

[Distributions] change dist
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
    model = TGP_avg
    sampler = test_sample
    evaluate_std = 'true'
    parallel_type = ROOT
    execute_on = final
  []
  [train_avg]
    type = EvaluateSurrogate
    model = TGP_avg
    sampler = train_sample
    evaluate_std = 'true'
    parallel_type = ROOT
    execute_on = final
  []
[]

[VectorPostprocessors]
  [hyperparams]
    type = TwoLayerGaussianProcessData
    tgp_name = 'TGP_avg'
    execute_on = final
  []
  [data]
    type = SamplerData
    sampler = test_sample
    execute_on = 'initial timestep_end'
  []
[]

[Trainers]
  [TGP_avg_trainer]
    type = TwoLayerGaussianProcessTrainer
    execute_on = timestep_end
    covariance_function = 'covar' #Choose a squared exponential for the kernel
    standardize_params = 'true' #Center and scale the training params
    standardize_data = 'true' #Center and scale the training data
    sampler = train_sample
    response = results/data:avg:value
    tune_parameters = 'covar:signal_variance covar:length_factor'
    num_iters = 1000
    batch_size = 20
    learning_rate = 0.005
  []
[]

[Surrogates]
  [TGP_avg]
    type = TwoLayerGaussianProcessSurrogate
    trainer = TGP_avg_trainer
  []
[]

[Covariance]
  [covar]
    type = SquaredExponentialCovariance
    signal_variance = 1.0 #Use a signal variance of 1 in the kernel
    noise_variance = 1e-6 #A small amount of noise can help with numerical stability
    length_factor = '1.0 1.0' #Select a length factor for each parameter (k and q)
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
    file_base = 'dgp'
  []
[]
