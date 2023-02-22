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
  [mc0]
    type = MonteCarlo
    num_rows = 15
    distributions = 'uniform_0'
    execute_on = INITIAL
  []
  [mc1]
    type = MonteCarlo
    num_rows = 15
    distributions = 'uniform_1'
    execute_on = INITIAL
  []
[]

[MultiApps]
  [runner0]
    type = SamplerFullSolveMultiApp
    sampler = mc0
    input_files = 'sub.i'
    mode = batch-reset
  []
  [runner1]
    type = SamplerFullSolveMultiApp
    sampler = mc1
    input_files = 'sub.i'
    mode = batch-reset
  []
[]

[Transfers]
  [runner0]
    type = SamplerParameterTransfer
    to_multi_app = runner0
    sampler = mc0
    parameters = 'BCs/left/value'
  []
  [runner1]
    type = SamplerParameterTransfer
    to_multi_app = runner1
    sampler = mc1
    parameters = 'BCs/right/value'
  []
  [data0]
    type = SamplerPostprocessorTransfer
    from_multi_app = runner0
    sampler = mc0
    to_vector_postprocessor = storage0
    from_postprocessor = average
  []
  [data1]
    type = SamplerPostprocessorTransfer
    from_multi_app = runner1
    sampler = mc1
    to_vector_postprocessor = storage1
    from_postprocessor = average
  []
[]

[VectorPostprocessors]
  [storage0]
    type = StochasticResults
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [storage1]
    type = StochasticResults
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [data0]
    type = SamplerData
    sampler = mc0
  []
  [data1]
    type = SamplerData
    sampler = mc1
  []
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
