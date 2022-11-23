[StochasticTools]
  auto_create_executioner = false
[]

[Samplers]
  [sample]
    type = CartesianProduct
    execute_on = PRE_MULTIAPP_SETUP
    linear_space_items = '1 1 2
                          0.1 0.1 2
                          0 1e-8 2'
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    ignore_solve_not_converge = true
  []
[]

[Transfers]
  [data]
    type = SamplerReporterTransfer
    from_multi_app = sub
    sampler = sample
    stochastic_reporter = storage
    from_reporter = 'pp/value vpp/vec constant/str constant/int'
  []
[]

[Controls]
  [runner]
    type = MultiAppSamplerControl
    multi_app = sub
    param_names = 'Reporters/constant/integer_values
                   real_val
                   Executioner/nl_rel_tol'
    sampler = sample
  []
[]

[Reporters]
  [storage]
    type = StochasticReporter
    execute_on = 'initial timestep_end'
    parallel_type = ROOT
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 0.01
[]

[Outputs]
  [out]
    type = JSON
  []
[]
