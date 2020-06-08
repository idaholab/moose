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
    num_rows = 6
    distributions = 'q_dist'
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
    param_names = 'Kernels/source/value'
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
  [gauss_process_avg]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    length_factor = '20000.0'
    distributions = 'q_dist'
    sampler = sample
    results_vpp = results
    results_vector = data:avg
  []
  [gauss_process_max]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    length_factor = '20000.0'
    distributions = 'q_dist'
    sampler = sample
    results_vpp = results
    results_vector = data:max
  []
[]

[Outputs]
  file_base = gauss_process_training
  [out]
    type = SurrogateTrainerOutput
    trainers = 'gauss_process_avg gauss_process_max'
    execute_on = FINAL
  []
[]
