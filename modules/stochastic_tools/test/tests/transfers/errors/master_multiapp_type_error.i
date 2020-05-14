[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 0
    upper_bound = 0.5
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 5
    distributions = 'uniform'
    execute_on = 'initial timestep_end'
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

[Outputs]
  execute_on = 'INITIAL TIMESTEP_END'
[]
