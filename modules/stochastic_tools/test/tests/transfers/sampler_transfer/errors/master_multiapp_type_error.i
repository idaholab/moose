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
  [uniform]
    type = UniformDistribution
    lower_bound = 1
    upper_bound = 2
  []
[]

[Samplers]
  [sample]
    type = MonteCarloSampler
    num_rows = 3
    distributions = 'uniform'
    execute_on = INITIAL # create random numbers on initial and use them for each timestep
  []
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    input_files = sub.i
    positions = '0 0 0'
  []
[]

[Transfers]
  [sub]
    type = SamplerParameterTransfer
    multi_app = sub
    sampler = sample
    parameters = 'BCs/left/value BCs/right/value'
    to_control = 'stochasticsub'
  []
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
