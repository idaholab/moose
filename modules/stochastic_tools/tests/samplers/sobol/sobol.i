[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  ny = 1
[]

[Variables]
  [./u]
  [../]
[]

[Distributions]
  [./d0]
    type = UniformDistribution
    lower_bound = 0
    upper_bound = 1
  [../]
  [./d1]
    type = UniformDistribution
    lower_bound = 10
    upper_bound = 11
  [../]
  [./d2]
    type = UniformDistribution
    lower_bound = 100
    upper_bound = 101
  [../]
[]

[Samplers]
  [./sample]
    type = SobolSampler
    n_samples = 4
    distributions = 'd0 d1 d2'
    execute_on = 'initial'
  [../]
[]

[VectorPostprocessors]
  [./data]
    type = SamplerData
    sampler = sample
    execute_on = 'initial'
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  execute_on = 'INITIAL'
  csv = true
[]
