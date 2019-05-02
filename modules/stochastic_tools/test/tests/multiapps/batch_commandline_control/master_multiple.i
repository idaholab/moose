[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Steady
[]

[Distributions]
  [uniform]
    type = UniformDistribution
    lower_bound = 5
    upper_bound = 10
  []
[]

[Samplers]
  [sample]
    type = MonteCarloSampler
    n_samples = 3
    distributions = 'uniform uniform'
    execute_on = 'PRE_MULTIAPP_SETUP'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    sampler = sample
    input_files = 'sub.i'
  []
[]

[Transfers]
  [data]
    type = SamplerPostprocessorTransfer
    multi_app = sub
    vector_postprocessor = storage
    postprocessor = size
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
  []
[]

[Controls]
  [cmdline]
    type = MultiAppCommandLineControl
    multi_app = sub
    sampler = sample
    param_names = 'Mesh/xmax Mesh/ymax'
  []
[]

[Outputs]
  [out]
    type = CSV
    execute_on = FINAL
  []
[]
