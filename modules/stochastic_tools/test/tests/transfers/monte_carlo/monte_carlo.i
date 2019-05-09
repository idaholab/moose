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
  [./uniform_left]
    type = UniformDistribution
    lower_bound = 0
    upper_bound = 0.5
  [../]
  [./uniform_right]
    type = UniformDistribution
    lower_bound = 1
    upper_bound = 2
  [../]
[]

[Samplers]
  [./sample]
    type = MonteCarloSampler
    n_samples = 5
    distributions = 'uniform_left uniform_right'
    execute_on = INITIAL # create random numbers on initial and use them for each timestep
  [../]
[]

[MultiApps]
  [./sub]
    type = SamplerTransientMultiApp
    input_files = sub.i
    sampler = sample
  [../]
[]

[Transfers]
  [./sub]
    type = SamplerTransfer
    multi_app = sub
    parameters = 'BCs/left/value BCs/right/value'
    to_control = 'stochastic'
    execute_on = INITIAL
    check_multiapp_execute_on = false
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 0.01
[]

[Problem]
  solve = false
  kernel_coverage_check = false
[]

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
[]
