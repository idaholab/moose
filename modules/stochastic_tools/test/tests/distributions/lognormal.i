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
  [./lognormal_test]
    type = LognormalDistribution
    location = -0.371
    scale = 0.52
  [../]
[]

[Postprocessors]
  [./cdf]
    type = TestDistributionPostprocessor
    distribution = lognormal_test
    value = 0.6
    method = cdf
    execute_on = initial
  [../]
  [./pdf]
    type = TestDistributionPostprocessor
    distribution = lognormal_test
    value = 0.6
    method = pdf
    execute_on = initial
  [../]
  [./quantile]
    type = TestDistributionPostprocessor
    distribution = lognormal_test
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
