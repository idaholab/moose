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
    lower_bound = 1980
    upper_bound = 2017
  [../]
[]

[Samplers]
  [./sample]
    type = MonteCarloSampler
    n_samples = 10
    distributions = 'uniform'
    execute_on = 'initial'
  [../]
[]

[UserObjects]
  [./test]
    type = TestSampler
    sampler = sample
    test_type = THREAD
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
[]
