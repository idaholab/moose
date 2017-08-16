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
    lower_bound = -42
    upper_bound = 42
  [../]
[]

[Samplers]
  [./sample]
    type = MonteCarloSampler
    n_samples = 10
    distributions = 'uniform'
    execute_on = 'initial' # Create random numbers on initial only, they remain the same with time.
  [../]
[]

[VectorPostprocessors]
  [./data]
    type = SamplerData
    sampler = sample
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
  csv = true
[]
