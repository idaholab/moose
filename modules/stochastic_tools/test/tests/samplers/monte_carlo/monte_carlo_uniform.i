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
  [./uniform]
    type = UniformDistribution
    lower_bound = 1
    upper_bound = 7
  [../]
[]

[Samplers]
  [./sample]
    type = MonteCarloSampler
    n_samples = 10
    distributions = 'uniform'
    execute_on = 'initial timestep_end'
  [../]
[]

[VectorPostprocessors]
  [./data]
    type = SamplerData
    sampler = sample
    execute_on = 'initial timestep_end'
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
  execute_on = 'INITIAL TIMESTEP_END'
  csv = true
[]
