[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  [uniform_left]
    type = Uniform
    lower_bound = 0
    upper_bound = 0.5
  []
  [uniform_right]
    type = Uniform
    lower_bound = 1
    upper_bound = 2
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    distributions = 'uniform_left uniform_right'
    num_rows = 3
    seed = 2011
  []
  [resample]
    type = MonteCarlo
    distributions = 'uniform_left uniform_right'
    num_rows = 3
    seed = 2013
  []
  [sobol]
    type = Sobol
    sampler_a = sample
    sampler_b = resample
  []
  [mc]
    type = MonteCarlo
    num_rows = 5
    distributions = 'uniform_left uniform_right'
    execute_on = INITIAL # create random numbers on initial and use them for each timestep
  []
[]

[MultiApps]
  [sobol]
    type = SamplerTransientMultiApp
    input_files = sub.i
    sampler = sobol
  []
  [mc]
    type = SamplerTransientMultiApp
    input_files = sub.i
    sampler = mc
  []
[]

[Transfers]
  [sobol]
    type = SamplerParameterTransfer
    to_multi_app = sobol
    sampler = sobol
    parameters = 'BCs/left/value BCs/right/value'
  []
  [sobol_data]
    type = SamplerPostprocessorTransfer
    from_multi_app = sobol
    sampler = sobol
    to_vector_postprocessor = storage
    from_postprocessor = avg
  []

  [mc]
    type = SamplerParameterTransfer
    to_multi_app = mc
    sampler = mc
    parameters = 'BCs/left/value BCs/right/value'
  []
  [mc_data]
    type = SamplerPostprocessorTransfer
    from_multi_app = mc
    sampler = mc
    to_vector_postprocessor = storage
    from_postprocessor = "avg max"
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
    parallel_type = REPLICATED
  []
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
