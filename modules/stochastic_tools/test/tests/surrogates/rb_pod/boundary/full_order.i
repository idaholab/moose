[StochasticTools]
[]

[Distributions]
  [k_dist]
    type = Uniform
    lower_bound = 2.5
    upper_bound = 7.5
  []
  [alpha_dist]
    type = Uniform
    lower_bound = 2.5
    upper_bound = 7.5
  []
  [S_dist]
    type = Uniform
    lower_bound = 2.5
    upper_bound = 7.5
  []
  [bc_dist]
    type = Uniform
    lower_bound = 0
    upper_bound = 1
  []
[]

[Samplers]
  [sample]
    type = LatinHypercube
    distributions = 'k_dist alpha_dist S_dist bc_dist'
    num_rows = 1000
    num_bins = 10
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    execute_on = 'timestep_begin'
  []
[]

[Transfers]
  [quad]
    type = SamplerParameterTransfer
    multi_app = runner
    sampler = sample
    parameters = 'Materials/k/prop_values Materials/alpha/prop_values Kernels/source/value BCs/left/value'
    to_control = 'stochastic'
    execute_on = 'timestep_begin'
  []
  [results]
    type = SamplerPostprocessorTransfer
    multi_app = runner
    sampler = sample
    to_vector_postprocessor = results
    from_postprocessor = 'max'
  []
[]

[VectorPostprocessors]
  [results]
    type = StochasticResults
  []
[]

[Outputs]
  csv = true
[]
