[StochasticTools]
[]

[Distributions]
  [S_dist]
    type = Uniform
    lower_bound = 0
    upper_bound = 20
  []
  [D_dist]
    type = Uniform
    lower_bound = 0
    upper_bound = 20
  []
  [L_dist]
    type = Uniform
    lower_bound = 1
    upper_bound = 10
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 50
    distributions = 'S_dist D_dist L_dist'
    execute_on = PRE_MULTIAPP_SETUP
    # min_procs_per_row = 4
  []
[]

[VariableMappings]
  [rb_mapping]
    type = DEIMRBMapping
    solution_storage = parallel_storage
    variables = 'solution residual jacobian'
    num_modes_to_compute = '30 4 10'
    extra_slepc_options = "-svd_monitor_all"
    jac_index_name = 'jacobian_storage/indices'
  []
[]

[MultiApps]
  [worker]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    mode = batch-reset
    # min_procs_per_app = 4
  []
[]

[Transfers]

  [solution_transfer]
    type = SerializedSnapshotTransfer
    parallel_storage = parallel_storage
    from_multi_app = worker
    sampler = sample
    solution_container = solution_storage
    residual_container = residual_storage
    jacobian_container = jacobian_storage
    serialize_on_root = true
  []
  # We are only transferring the one copy of the indices. We assume that the
  # order, number, anything does not change from one run to another. Breaking
  # this assumption would not allow M/DEIM to work in the current configuration.
  [jac_indices]
    type = MultiAppReporterTransfer
    from_multi_app = worker
    from_reporters = 'jacobian_storage/indices'
    to_reporters = 'jacobian_storage/indices'
    subapp_index = 0
  []
[]

[Controls]
  [cmd_line]
    type = MultiAppSamplerControl
    multi_app = worker
    sampler = sample
    param_names = 'S D L'
  []
[]

[Reporters]
  [parallel_storage]
    type = ParallelSolutionStorage
    variables = 'solution residual jacobian'
    outputs = none
  []
  [build_mapping]
    type = MappingReporter
    mapping = rb_mapping
    variables = "solution residual jacobian"
    build_all_mappings_only = true
    execute_on = TIMESTEP_END # To make sure the trainer sees the results on
    outputs = out
  []
  # Allow for the transfer of the indices. Create the reporter type needed.
  [jacobian_storage]
    type = JacobianContainer
    jac_indices_reporter_name = indices
    execute_on = 'NONE'
    outputs = none
  []
[]

[Outputs]
  [mapping]
    type = MappingOutput
    mappings = rb_mapping
    execute_on = FINAL
  []
  [out]
    type = JSON
    execute_on = TIMESTEP_END
    execute_system_information_on = none
  []
[]
