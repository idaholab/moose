[StochasticTools]
[]

[Distributions]
  [weibull]
    type = Weibull
    scale = 1
    shape = 5
    location = 0
  []
  [uniform]
    type = Uniform
    lower_bound = 1
    upper_bound = 7
  []
[]

[Samplers]
  [sample]
    type = Metropolis
    num_rows = 3
    distributions = 'weibull uniform'
    inputs_vpp = data1
    inputs_names = 'sample_0 sample_1'
    proposal_std = '0.15 0.5'
    initial_values = '0.5 3.5'
    execute_on = timestep_end # 'initial timestep_end'
    # seed = 105
  []
[]

[VectorPostprocessors]
  [data]
    type = SamplerData
    sampler = sample
    execute_on = timestep_begin # 'initial timestep_end'
    # sample_method = 'get_global_samples'
    # parallel_type = 'distributed'
  []
  [data1]
    type = SamplerData
    sampler = sample
    execute_on = timestep_end # 'initial timestep_end'
    # sample_method = 'get_global_samples'
    # parallel_type = 'distributed'
  []
[]

[Executioner]
  type = Transient
  num_steps = 5
[]

[Outputs]
  execute_on = timestep_end
  csv = true
[]
