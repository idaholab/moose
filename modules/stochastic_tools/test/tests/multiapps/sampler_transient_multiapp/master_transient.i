[Mesh]
  type = GeneratedMesh
  dim = 1
[]

[Problem]
  kernel_coverage_check = false
  solve = false
[]

[Distributions]
  [./uniform_0]
    type = UniformDistribution
    lower_bound = 0.1
    upper_bound = 0.3
  [../]
[]

[Samplers]
  [./mc]
    type = MonteCarloSampler
    n_samples = 5
    distributions = 'uniform_0'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[MultiApps]
  [./runner]
    type = SamplerTransientMultiApp
    sampler = mc
    input_files = 'sub.i'
  [../]
[]
