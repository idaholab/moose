[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Uniform
    lower_bound = 0
    upper_bound = 20
  []
  [q_dist]
    type = Uniform
    lower_bound = 7000
    upper_bound = 13000
  []
  [L_dist]
    type = Uniform
    lower_bound = 0.0
    upper_bound = 0.1
  []
  [Tinf_dist]
    type = Uniform
    lower_bound = 270
    upper_bound = 330
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 500
    distributions = 'k_dist q_dist L_dist Tinf_dist'
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = sample
    param_names = 'Materials/conductivity/prop_values Kernels/source/value Mesh/xmax BCs/right/value'
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    sampler = sample
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
  [GP_avg]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    covariance_function = 'rbf'
    standardize_params = 'true'               #Center and scale the training params
    standardize_data = 'true'                 #Center and scale the training data
    sampler = sample
    response = results/data:avg:value
    tune_parameters = 'rbf:signal_variance rbf:length_factor'
    tuning_min = ' 1e-9 1e-3'
    tuning_max = ' 100  100'
    num_iters = 200
    learning_rate = 0.005
  []
[]

[Covariance]
  [rbf]
    type=SquaredExponentialCovariance
    noise_variance = 1e-3                     #A small amount of noise can help with numerical stability
    signal_variance = 1
    length_factor = '0.038971 0.038971 0.038971 0.038971' #Select a length factor for each parameter
  []
[]

[Outputs]
  file_base = GP_training_normal
  [out]
    type = SurrogateTrainerOutput
    trainers = 'GP_avg'
    execute_on = FINAL
  []
[]
