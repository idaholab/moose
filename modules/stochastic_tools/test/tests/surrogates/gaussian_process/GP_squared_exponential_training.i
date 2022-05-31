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
[]

[Trainers]
  [GP_avg_trainer]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    covariance_function = 'covar'             #Choose a squared exponential for the kernel
    standardize_params = 'true'               #Center and scale the training params
    standardize_data = 'true'                 #Center and scale the training data
    sampler = train_sample
    response = results/data:avg:value
  []
[]

[Covariance]
  [covar]
    type=SquaredExponentialCovariance
    signal_variance = 1                       #Use a signal variance of 1 in the kernel
    noise_variance = 1e-6                     #A small amount of noise can help with numerical stability
    length_factor = '0.38971 0.38971'         #Select a length factor for each parameter (k and q)
  []
[]

[Outputs]
  file_base = gauss_process_training
  [out]
    type = SurrogateTrainerOutput
    trainers = 'GP_avg_trainer'
    execute_on = FINAL
  []
[]
