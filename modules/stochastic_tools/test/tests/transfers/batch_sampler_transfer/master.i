[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Distributions]
  [uniform_0]
    type = UniformDistribution
    lower_bound = 100
    upper_bound = 200
  []
  [uniform_1]
    type = UniformDistribution
    lower_bound = 1
    upper_bound = 2
  []
[]

[Samplers]
  [mc]
    type = MonteCarloSampler
    n_matrices = 3
    n_samples = 5
    distributions = 'uniform_0 uniform_1'
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
    mode = batch-reset
  []
[]

[Transfers]
  [runner]
    type = SamplerTransfer
    multi_app = runner
    parameters = 'BCs/left/value BCs/right/value'
    to_control = 'stochastic'
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
  [data]
    type = SamplerData
    sampler = mc
  []
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
