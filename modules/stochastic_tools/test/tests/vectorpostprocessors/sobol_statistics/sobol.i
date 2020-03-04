[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Distributions/uniform]
  type = UniformDistribution
  lower_bound = 0
  upper_bound = 1
[]

[Samplers/sample]
  type = SobolSampler
  num_rows = 1000
  distributions = 'uniform uniform uniform uniform uniform uniform'
  execute_on = 'initial'
[]

[MultiApps/sub]
    type = SamplerFullSolveMultiApp
    input_files = gfunction.i
    mode = batch-restore
    sampler = sample
    execute_on = INITIAL
[]

[Transfers]
  [sub]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'Functions/g/x_vector'
    to_control = 'stochastic'
  []
  [data]
    type = SamplerPostprocessorTransfer
    multi_app = sub
    sampler = sample
    to_vector_postprocessor = results
    from_postprocessor = y
  []
[]

[VectorPostprocessors]
  [results]
    type = StochasticResults
    samplers = sample
    execute_on = INITIAL
    outputs = none
  []
  [sobol]
    type = SobolStatistics
    sampler = sample
    results = results
    execute_on = FINAL
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  execute_on = 'FINAL'
  csv = true
  [perf]
    type = PerfGraphOutput
    level = 4
  []
[]
