[StochasticTools]
[]

[Samplers]
  [directperturbation]
    type = DirectPerturbationSampler
    nominal_parameter_values = '0.1 0.2 0.3'
    relative_perturbation_intervals = '0.1 0.2 0.3'
    perturbation_method = central_difference
  []
[]

[VectorPostprocessors/data]
  type = SamplerData
  sampler = directperturbation
  execute_on = 'initial timestep_end'
[]

[Outputs]
  csv = true
[]
