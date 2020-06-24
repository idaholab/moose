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
    kernel_function = 'matern_half_int'
    standardize_params = 'true'
    standardize_data = 'true'
    p = 2
    signal_variance = 1
    noise_variance = 1e-6
    length_factor = '0.551133 0.551133'
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



[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
