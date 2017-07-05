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
  [./weibull]
    type = WeibullDistribution
    scale = 1
    shape = 5
  [../]
[]

[Samplers]
  [./sample]
    type = MonteCarloSampler
    n_samples = 10
    distributions = 'weibull'
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
