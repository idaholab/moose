[StochasticTools]
[]

# [Distributions]
#   [k_dist]
#     type = TruncatedNormal
#     mean = 5
#     standard_deviation = 2
#     lower_bound = 0
#   []
#   [q_dist]
#     type = TruncatedNormal
#     mean = 10000
#     standard_deviation = 500
#     lower_bound = 0
#   []
#   [L_dist]
#     type = TruncatedNormal
#     mean = 0.03
#     standard_deviation = 0.01
#     lower_bound = 0
#   []
#   [Tinf_dist]
#     type = TruncatedNormal
#     mean = 300
#     standard_deviation = 10
#     lower_bound = 0
#   []
# []

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
    noise_variance = 1e-3                     #A small amount of noise can help with numerical stability
    signal_variance = 1
    length_factor = '0.038971 0.038971 0.038971 0.038971' #Select a length factor for each parameter (k and q)
    # signal_variance = 1.98641                        #Use a signal variance of 1 in the kernel
    # length_factor = '0.912497 0.192563 0.784225 1.21264' #Select a length factor for each parameter (k and q)
  []
  [rbf2]
    type=SquaredExponentialCovariance
    noise_variance = 1e-3                     #A small amount of noise can help with numerical stability
    signal_variance = 1
    length_factor = '0.038971 0.038971 0.038971 0.038971' #Select a length factor for each parameter (k and q)
    # signal_variance = 1.98641                      #Use a signal variance of 1 in the kernel
    # length_factor = '0.912497 0.192563 0.784225 1.21264' #Select a length factor for each parameter (k and q)
  []
[]

[Outputs]
  file_base = GP_training_normal
  [out]
    type = SurrogateTrainerOutput
    trainers = 'GP_avg GP_max'
    execute_on = FINAL
  []
[]
