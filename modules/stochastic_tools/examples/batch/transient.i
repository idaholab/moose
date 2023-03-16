[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 1
    upper_bound = 9
  []
[]

[Samplers]
  [mc]
    type = MonteCarlo
    num_rows = 10
    distributions = 'uniform uniform'
  []
[]

[Executioner]
  type = Transient
  num_steps = 10
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = mc
    input_files = 'sub.i'
    execute_on = 'INITIAL TIMESTEP_END'
    mode = batch-restore
  []
[]

[Transfers]
  [runner]
    type = SamplerParameterTransfer
    to_multi_app = runner
    parameters = 'BCs/left/value BCs/right/value'
    sampler = mc
  []
  [data]
    type = SamplerPostprocessorTransfer
    from_multi_app = runner
    to_vector_postprocessor = storage
    from_postprocessor = average
    sampler = mc
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
  []
[]

[Postprocessors]
  [total]
    type = MemoryUsage
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [per_proc]
    type = MemoryUsage
    value_type = "average"
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [max_proc]
    type = MemoryUsage
    value_type = "max_process"
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Outputs]
  csv = true
  perf_graph = true
[]
