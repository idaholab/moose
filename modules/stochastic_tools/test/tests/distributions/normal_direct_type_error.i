[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
  ny = 1
[]

[Variables]
  [u]
  []
[]

[Distributions]
  [this_is_the_wrong_type]
    type = UniformDistribution
    lower_bound = 0
    upper_bound = 1
  []
[]

[Postprocessors]
  [cdf]
    type = TestDistributionDirectPostprocessor
    distribution = this_is_the_wrong_type
    value = 0
    method = cdf
    execute_on = initial
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
  execute_on = 'INITIAL'
  csv = true
[]
