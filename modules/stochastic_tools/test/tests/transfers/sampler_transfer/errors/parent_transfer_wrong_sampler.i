[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  [uniform]
    type = Uniform
    lower_bound = 1
    upper_bound = 2
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 3
    distributions = 'uniform'
    execute_on = INITIAL # create random numbers on initial and use them for each timestep
  []

  [wrong]
    type = MonteCarlo
    num_rows = 3
    distributions = 'uniform'
    execute_on = INITIAL # create random numbers on initial and use them for each timestep
  []
[]

[MultiApps]
  [sub]
    type = SamplerTransientMultiApp
    sampler = sample
    input_files = sub.i
  []
[]

[Transfers]
  [sub]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = wrong
    parameters = 'BCs/left/value BCs/right/value'
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
