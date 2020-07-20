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
    num_rows = 100
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
    type = MultiAppCommandLineControl
    multi_app = sub
    sampler = train_sample
    param_names = 'Materials/conductivity/prop_values Kernels/source/value'
  []
[]

[Transfers]
  [data]
    type = SamplerPostprocessorTransfer
    multi_app = sub
    sampler = train_sample
    to_vector_postprocessor = results
    from_postprocessor = 'avg'
  []
[]

[VectorPostprocessors]
  [results]
    type = StochasticResults
  []
  [samp_avg]
    type = GaussianProcessTester
    model = GP_avg
    sampler = test_sample
    output_samples = true
    execute_on = final
  []
  [train_avg]
    type = GaussianProcessTester
    model = GP_avg
    sampler = train_sample
    output_samples = true
    execute_on = final
  []
[]

[Trainers]
  [GP_avg_trainer]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    covariance_function = 'covar'   #Choose a Matern with half-integer argument for the kernel
    standardize_params = 'true'           #Center and scale the training params
    standardize_data = 'true'             #Center and scale the training data
    distributions = 'k_dist q_dist'
    sampler = train_sample
    results_vpp = results
    results_vector = data:avg
  []
[]

[Surrogates]
  [GP_avg]
    type = GaussianProcess
    trainer = GP_avg_trainer
  []
[]

[Covariance]
  [covar]
    type=MaternHalfIntCovariance
    p = 2                                 #Define the exponential factor
    signal_variance = 1                       #Use a signal variance of 1 in the kernel
    noise_variance = 1e-6                     #A small amount of noise can help with numerical stability
    length_factor = '0.551133 0.551133'       #Select a length factor for each parameter (k and q)
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
