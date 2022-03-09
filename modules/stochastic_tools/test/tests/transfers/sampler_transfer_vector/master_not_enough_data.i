[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  [uniform_prop_a]
    type = Uniform
    lower_bound = 1980
    upper_bound = 1981
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 5
    distributions = 'uniform_prop_a'
    execute_on = 'initial timestep_end' # create new random numbers on initial and timestep_end
  []
[]

[MultiApps]
  [sub]
    type = SamplerTransientMultiApp
    input_files = sub.i
    sampler = sample
    execute_on = 'initial timestep_end'
  []
[]

[Transfers]
  [sub]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = sample
    parameters = 'Materials/mat/prop_values'
    to_control = 'stochastic'
    execute_on = 'initial timestep_end'
    check_multiapp_execute_on = false
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 0.01
[]

[Outputs]
  execute_on = 'initial timestep_end'
[]
