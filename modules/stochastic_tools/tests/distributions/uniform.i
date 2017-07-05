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
    lower_bound = 5
    upper_bound = 10
  [../]
[]

[Postprocessors]
  [./cdf]
    type = TestDistributionPostprocessor
    distribution = uniform
    value = 7.5
    method = cdf
    execute_on = initial
  [../]
  [./pdf]
    type = TestDistributionPostprocessor
    distribution = uniform
    value = 7.5
    method = pdf
    execute_on = initial
  [../]
  [./quantile]
    type = TestDistributionPostprocessor
    distribution = uniform
    value = 0.5
    method = quantile
    execute_on = initial
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
