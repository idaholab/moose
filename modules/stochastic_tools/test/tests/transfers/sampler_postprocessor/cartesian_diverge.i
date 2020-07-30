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
    multi_app = sub
    sampler = cartesian
    to_vector_postprocessor = storage
    from_postprocessor = avg
    # If this is true, the transfer will omit values from sub apps that did not converge
    skip_solve_not_converge_value = false # Default: false
    # If this parameter is set,
    # the transfer will transfer this value for sub apps that did not converge,
    # otherwise it will transfer whatever the last computed values is
    # solve_not_converge_value = 0 # Default: none
  []
[]

[VectorPostprocessors]
  [storage]
    type = StochasticResults
  []
[]

[Controls]
  [cmdline]
    type = MultiAppCommandLineControl
    multi_app = sub
    sampler = cartesian
    param_names = 'Executioner/nl_rel_tol'
  []
[]

[Outputs]
  csv = true
  execute_on = 'TIMESTEP_END'
[]
