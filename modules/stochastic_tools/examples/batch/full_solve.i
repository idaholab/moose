[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Distributions]
  [uniform]
    type = UniformDistribution
    lower_bound = 1
    upper_bound = 9
  []
[]

[Samplers]
  [mc]
    type = MonteCarloSampler
    n_samples = 10
    distributions = 'uniform uniform'
  []
[]

[Executioner]
  type = Steady
[]

[MultiApps]
  [runner]
    type = SamplerFullSolveMultiApp
    sampler = mc
    input_files = 'sub.i'
    mode = batch-restore
  []
[]

[Transfers]
  [runner]
    type = SamplerTransfer
    multi_app = runner
    parameters = 'BCs/left/value BCs/right/value'
    to_control = receiver
  []
  [data]
    type = SamplerPostprocessorTransfer
    multi_app = runner
    vector_postprocessor = storage
    postprocessor = average
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
