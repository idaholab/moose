[StochasticTools]
[]

[Distributions]
  [mu1_dist]
    type = Uniform
    lower_bound = 0.21
    upper_bound = 0.39
  []
  [mu2_dist]
    type = Uniform
    lower_bound = 6.3
    upper_bound = 11.7
  []
[]

[Samplers]
  [sample]
    type = Quadrature
    order = 10
    distributions = 'mu1_dist mu2_dist'
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

[Transfers]
  [parameters]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'Kernels/nonlin_function/mu1 Kernels/nonlin_function/mu2'
    to_control = 'stochastic'
  []
  [results]
    type = SamplerPostprocessorTransfer
    multi_app = sub
    sampler = sample
    to_vector_postprocessor = results
    from_postprocessor = 'max min average'
  []
[]

[VectorPostprocessors]
  [results]
    type = StochasticResults
  []
[]

[Trainers]
  [poly_chaos_max]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 10
    distributions = 'mu1_dist mu2_dist'
    sampler = sample
    results_vpp = results
    results_vector = results:max
  []
  [poly_chaos_min]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 10
    distributions = 'mu1_dist mu2_dist'
    sampler = sample
    results_vpp = results
    results_vector = results:min
  []
  [poly_chaos_avg]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 10
    distributions = 'mu1_dist mu2_dist'
    sampler = sample
    results_vpp = results
    results_vector = results:average
  []
[]

[Outputs]
  file_base = poly_chaos_training
  [out]
    type = SurrogateTrainerOutput
    trainers = 'poly_chaos_max poly_chaos_min poly_chaos_avg'
    execute_on = FINAL
  []
[]
