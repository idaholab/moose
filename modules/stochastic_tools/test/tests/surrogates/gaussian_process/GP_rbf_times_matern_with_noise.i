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
  []
[]

[Surrogates]
  [GP_avg]
    type = GaussianProcessSurrogate
    trainer = GP_avg_trainer
  []
[]

[Covariance]
  [rbf]
    type = SquaredExponentialCovariance
    signal_variance = 1 #Use a signal variance of 1 in the kernel
    noise_variance = 0 #Must be zero: noise cannot live inside a Product sub-kernel
                       #(see 'noise_kernel' below for where it actually goes)
    length_factor = '0.38971 0.38971' #Select a length factor for each parameter (k and q)
  []
  [matern_half]
    type = MaternHalfIntCovariance
    p = 2 #Define the exponential factor
    signal_variance = 1 #Use a signal variance of 1 in the kernel
    noise_variance = 0 #Must be zero, for the same reason as 'rbf' above
    length_factor = '0.551133 0.551133' #Select a length factor for each parameter (k and q)
  []
  [rbf_x_matern]
    type = CovarianceCombiner
    covariance_functions = 'rbf matern_half' #The two kernels being multiplied
    operation = Product #K = K_rbf .* K_matern, element-wise
  []
  [noise_kernel]
    type = LinearCovariance
    signal_variance = 0 #Zeroed so this kernel contributes nothing but noise
    bias_variance = 0 #Zeroed so this kernel contributes nothing but noise
    noise_variance = 1e-6 #All observation/numerical-stability noise lives here
    c = '0 0' #Unused since signal_variance = 0, but required by the kernel; one
              #entry per input parameter (k and q)
  []
  [covar]
    type = CovarianceCombiner
    covariance_functions = 'rbf_x_matern noise_kernel' #Product term + pure noise term
    operation = Sum #K = (K_rbf .* K_matern) + noise_kernel
                    #All hyperparameters of rbf, matern_half, and noise_kernel
                    #are tuned jointly by GP_avg_trainer
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
