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
  [./normal_test]
    type = NormalDistribution
    mean =  0
    standard_deviation = 1
  [../]
[]

[Postprocessors]
  [./cdf]
    type = TestDistributionPostprocessor
    distribution = normal_test
    value = 0
    method = cdf
    execute_on = initial
  [../]
  [./pdf]
    type = TestDistributionPostprocessor
    distribution = normal_test
    value = 0
    method = pdf
    execute_on = initial
  [../]
  [./quantile]
    type = TestDistributionPostprocessor
    distribution = normal_test
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
