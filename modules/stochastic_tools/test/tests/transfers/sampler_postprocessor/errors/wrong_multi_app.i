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
    num_rows = 3
    distributions = 'uniform_left uniform_right'
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
  [runner]
    type = SamplerParameterTransfer
    to_multi_app = sub
    sampler = sample
    parameters = 'BCs/left/value BCs/right/value'
    execute_on = INITIAL
    check_multiapp_execute_on = false
  []
  [data]
    type = SamplerPostprocessorTransfer
    from_multi_app = sub
    sampler = sample
    to_vector_postprocessor = storage
    from_postprocessor = avg
    execute_on = timestep_end
    check_multiapp_execute_on = false
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
  dt = 0.01
[]
