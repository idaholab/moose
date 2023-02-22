[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  [uniform_left]
    type = Uniform
    lower_bound = 0
    upper_bound = 0.5
  []
  [uniform_right]
    type = Uniform
    lower_bound = 1
    upper_bound = 2
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 10
    distributions = 'uniform_left uniform_right'
    execute_on = INITIAL # create random numbers on initial and use them for each timestep
  []
[]

[MultiApps]
  [sub]
    type = SamplerTransientMultiApp
    input_files = sub.i
    sampler = sample
  []
[]

[Transfers]
  [sub]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = sample
    parameters = 'BCs/left/value BCs/right/value'
    execute_on = INITIAL
    check_multiapp_execute_on = false
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
