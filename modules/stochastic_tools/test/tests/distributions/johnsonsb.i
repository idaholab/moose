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
  [./johnsonsb_test]
    type = JohnsonSBDistribution
    a = 1
    b = 2
    alpha_1 = 1
    alpha_2 = 2
  [../]
[]

[Postprocessors]
  [./cdf]
    type = TestDistributionPostprocessor
    distribution = johnsonsb_test
    value = 1.5
    method = cdf
    execute_on = initial
  [../]
  [./pdf]
    type = TestDistributionPostprocessor
    distribution = johnsonsb_test
    value = 1.5
    method = pdf
    execute_on = initial
  [../]
  [./quantile]
    type = TestDistributionPostprocessor
    distribution = johnsonsb_test
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
