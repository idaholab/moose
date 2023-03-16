[StochasticTools]
  auto_create_executioner = false
[]

[Distributions]
  [uniform_left]
    type = Uniform
    lower_bound = 1
    upper_bound = 2
  []
  [uniform_right]
    type = Uniform
    lower_bound = 10
    upper_bound = 20
  []
  [uniform_prop_a]
    type = Uniform
    lower_bound = 1980
    upper_bound = 1981
  []
  [uniform_prop_b]
    type = Uniform
    lower_bound = 1949
    upper_bound = 1950
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 5
    distributions = 'uniform_left uniform_prop_a uniform_prop_b uniform_right'
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
    parameters = 'BCs/left/value[0] Materials/mat/prop_values[1,2] BCs/right/value[3]'
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
