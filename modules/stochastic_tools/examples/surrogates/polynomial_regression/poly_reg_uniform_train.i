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
    num_rows = 10000
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
  [poly_reg_avg]
    type = PolynomialRegressionTrainer
    execute_on = timestep_end
    max_degree = 3
    sampler = sample
    results_vpp = results
    results_vector = data:avg
  []
  [poly_reg_max]
    type = PolynomialRegressionTrainer
    execute_on = timestep_end
    max_degree = 3
    sampler = sample
    results_vpp = results
    results_vector = data:max
  []
[]

[Outputs]
  csv = true
  file_base = poly_reg_training
  [out]
    type = SurrogateTrainerOutput
    trainers = 'poly_reg_avg poly_reg_max'
    execute_on = FINAL
  []
[]
