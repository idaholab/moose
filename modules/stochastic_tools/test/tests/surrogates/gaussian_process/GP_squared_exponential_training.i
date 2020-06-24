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
[]

[Trainers]
  [GP_avg_trainer]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    kernel_function = 'squared_exponential'
    standardize_params = 'true'
    standardize_data = 'true'
    signal_variance = 1
    noise_variance = 1e-6
    length_factor = '0.38971 0.38971'
    distributions = 'k_dist q_dist'
    sampler = train_sample
    results_vpp = results
    results_vector = data:avg
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
