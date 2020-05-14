[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Normal
    mean = 5
    standard_deviation = 2
  []
  [q_dist]
    type = Normal
    mean = 10000
    standard_deviation = 500
  []
  [L_dist]
    type = Normal
    mean = 0.03
    standard_deviation = 0.01
  []
  [Tinf_dist]
    type = Normal
    mean = 300
    standard_deviation = 10
  []
[]

[Samplers]
  [sample]
    type = Quadrature
    order = 10
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
  [poly_chaos_avg]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 10
    distributions = 'k_dist q_dist L_dist Tinf_dist'
    sampler = sample
    results_vpp = results
    results_vector = data:avg
  []
  [poly_chaos_max]
    type = PolynomialChaosTrainer
    execute_on = timestep_end
    order = 10
    distributions = 'k_dist q_dist L_dist Tinf_dist'
    sampler = sample
    results_vpp = results
    results_vector = data:max
  []
[]

[Outputs]
  file_base = poly_chaos_training
  [out]
    type = SurrogateTrainerOutput
    trainers = 'poly_chaos_avg poly_chaos_max'
    execute_on = FINAL
  []
[]
