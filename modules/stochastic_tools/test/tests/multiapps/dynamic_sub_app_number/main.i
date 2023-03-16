[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 1
    upper_bound = 2
  []
[]

[Samplers]
  [mc]
    type = TestDynamicNumberOfSubAppsSampler
    num_rows = 5
    distributions = 'uniform'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = mc
    input_files = 'sub.i'
    execute_on = 'TIMESTEP_BEGIN'
  []
[]

[Transfers]
  [runner]
    type = SamplerParameterTransfer
    to_multi_app = runner
    sampler = mc
    parameters = 'BCs/right/value'
  []
  [data]
    type = SamplerPostprocessorTransfer
    from_multi_app = runner
    sampler = mc
    to_vector_postprocessor = storage
    from_postprocessor = center
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  [out]
    type = JSON
    vectorpostprocessors_as_reporters = true
  []
[]
