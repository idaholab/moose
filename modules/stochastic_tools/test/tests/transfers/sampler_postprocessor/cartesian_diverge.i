[StochasticTools]
[]

[Samplers]
  [cartesian]
    type = CartesianProduct
    linear_space_items = '0 1e-6 5'
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [sub]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = cartesian
    # This parameter will allow the main app to continue if a solve does not converge
    ignore_solve_not_converge = true # Default: false
  []
[]

[Transfers]
  [data]
    type = SamplerPostprocessorTransfer
    from_multi_app = sub
    sampler = cartesian
    to_vector_postprocessor = storage
    from_postprocessor = avg
    # If this is false, the transfer will transfer NaN for sub apps that did not converge.
    # If this is true, the transfer will transfer whatever the last computed values are.
    keep_solve_fail_value = true # Default: false
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = sub
    sampler = cartesian
    param_names = 'Executioner/nl_rel_tol'
  []
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
