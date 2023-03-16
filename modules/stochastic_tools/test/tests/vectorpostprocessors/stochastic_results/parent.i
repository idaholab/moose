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
[]

[MultiApps]
  [sub]
    type = SamplerTransientMultiApp
    input_files = sub.i
    sampler = sobol
  []
[]

[Transfers]
  [runner]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = sobol
    parameters = 'BCs/left/value BCs/right/value'
  []
  [data]
    type = SamplerPostprocessorTransfer
    from_multi_app = sub
    sampler = sobol
    to_vector_postprocessor = storage
    from_postprocessor = avg
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
    parallel_type = DISTRIBUTED
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
