[StochasticTools]
[]

[Distributions]
  [uniform_0]
    type = Uniform
    lower_bound = 100
    upper_bound = 200
  []
  [uniform_1]
    type = Uniform
    lower_bound = 1
    upper_bound = 2
  []
[]

[Samplers]
  [mc]
    type = MonteCarlo
    num_rows = 15
    distributions = 'uniform_0 uniform_1'
    execute_on = INITIAL
  []
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = mc
    input_files = 'sub.i'
    mode = batch-reset
  []
[]

[Transfers]
  [runner]
    type = SamplerParameterTransfer
    to_multi_app = runner
    sampler = mc
    parameters = 'BCs/left/value BCs/right/value'
  []
  [data]
    type = SamplerPostprocessorTransfer
    from_multi_app = runner
    sampler = mc
    to_vector_postprocessor = storage
    from_postprocessor = average
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [data]
    type = SamplerData
    sampler = mc
  []
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
