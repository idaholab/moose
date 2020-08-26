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
    type = MultiAppCommandLineControl
    multi_app = sub
    sampler = sample
    param_names = 'Materials/conductivity/prop_values Kernels/source/value Mesh/xmax BCs/right/value'
  []
[]

[Transfers]
  [data]
    type = SamplerPostprocessorTransfer
    multi_app = sub
    sampler = sample
    to_vector_postprocessor = results
    from_postprocessor = 'avg max'
  []
[]

[VectorPostprocessors]
  [results]
    type = StochasticResults
  []
[]

[Trainers]
  [GP_avg]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    covariance_function = 'rbf'
    standardize_params = 'true'               #Center and scale the training params
    standardize_data = 'true'                 #Center and scale the training data
    distributions = 'k_dist q_dist L_dist Tinf_dist'
    sampler = sample
    results_vpp = results
    results_vector = data:avg
    tune = 'true'
    tao_options = '-tao_bncg_type gd'
    tune_parameters = ' signal_variance length_factor'
    tuning_min = ' 1e-9 1e-3'
    tuning_max = ' 100  100'
    show_tao = 'true'
  []
  [GP_max]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    covariance_function = 'rbf2'
    standardize_params = 'true'               #Center and scale the training params
    standardize_data = 'true'                 #Center and scale the training data
    distributions = 'k_dist q_dist L_dist Tinf_dist'
    sampler = sample
    results_vpp = results
    results_vector = data:max
    tune = 'true'
    tao_options = '-tao_bncg_type gd'
    tune_parameters = ' signal_variance length_factor'
    tuning_min = ' 1e-9 1e-3'
    tuning_max = ' 100  100'
    show_tao = 'true'
  []
[]

[Covariance]
  [rbf]
    type=SquaredExponentialCovariance
    signal_variance = 1                       #Use a signal variance of 1 in the kernel
    noise_variance = 1e-3                     #A small amount of noise can help with numerical stability
    length_factor = '0.038971 0.038971 0.038971 0.038971' #Select a length factor for each parameter (k and q)
  []
  [rbf2]
    type=SquaredExponentialCovariance
    signal_variance = 1                       #Use a signal variance of 1 in the kernel
    noise_variance = 1e-3                     #A small amount of noise can help with numerical stability
    length_factor = '0.038971 0.038971 0.038971 0.038971' #Select a length factor for each parameter (k and q)
  []
[]

[Outputs]
  file_base = GP_training_uniform
  [out]
    type = SurrogateTrainerOutput
    trainers = 'GP_avg GP_max'
    execute_on = FINAL
  []
[]
