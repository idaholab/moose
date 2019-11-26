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
    lower_bound = 2
    upper_bound = 4
  []
[]

[Samplers]
  [mc]
    type = MonteCarloSampler
    num_rows = 5
    distributions = 'uniform uniform'
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[MultiApps]
  [runner]
    type = SamplerTransientMultiApp
    sampler = mc
    input_files = 'sub.i'
    execute_on = 'INITIAL TIMESTEP_END'
    mode = batch-restore
  []
[]

[Transfers]
  [runner]
    type = SamplerParameterTransfer
    multi_app = runner
    sampler = mc
    parameters = 'BCs/left/value BCs/right/value'
    to_control = 'stochastic'
  []
  [data]
    type = SamplerPostprocessorTransfer
    multi_app = runner
    sampler = mc
    vector_postprocessor = storage
    postprocessor = average
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
    samplers = mc
  []
[]

[Outputs]
  csv = true
[]
