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
    lower_bound = 0.1
    upper_bound = 0.3
  []
[]

[Samplers]
  [mc]
    type = MonteCarloSampler
    n_samples = 5
    distributions = 'uniform_0'
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
    mode = batch
  []
[]

[Transfers]
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

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
