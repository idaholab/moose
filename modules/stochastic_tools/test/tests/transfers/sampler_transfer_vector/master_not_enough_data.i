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
  [./uniform_prop_a]
    type = UniformDistribution
    lower_bound = 1980
    upper_bound = 1981
  [../]
[]

[Samplers]
  [./sample]
    type = MonteCarloSampler
    n_samples = 5
    distributions = 'uniform_prop_a'
    execute_on = 'initial timestep_end' # create new random numbers on initial and timestep_end
  [../]
[]

[MultiApps]
  [./sub]
    type = SamplerTransientMultiApp
    input_files = sub.i
    sampler = sample
    execute_on = 'initial timestep_end'
  [../]
[]

[Transfers]
  [./sub]
    type = SamplerTransfer
    multi_app = sub
    parameters = 'Materials/mat/prop_values'
    to_control = 'stochastic'
    execute_on = 'initial timestep_end'
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
  execute_on = 'initial timestep_end'
[]
