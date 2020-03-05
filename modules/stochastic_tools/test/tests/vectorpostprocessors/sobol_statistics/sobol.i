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
  num_rows = 10000
  distributions = 'uniform uniform uniform uniform uniform uniform'
  execute_on = 'initial'
[]

[VectorPostprocessors]
  [results]
    type = GFunction
    sampler = sample
    q_vector = '0 0.5 3 9 99 99'
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
[]
