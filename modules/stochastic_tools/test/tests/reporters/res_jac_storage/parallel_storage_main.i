[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
  []
[]

[Problem]
  solve = false
[]
[Executioner]
  type = Steady
[]

[Distributions]
  [S_dist]
    type = Uniform
    lower_bound = 0
    upper_bound = 10
  []
  [D_dist]
    type = Uniform
    lower_bound = 0
    upper_bound = 10
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 4
    distributions = 'S_dist D_dist'
    execute_on = PRE_MULTIAPP_SETUP
    seed = 0
  []
[]

[MultiApps]
  [worker]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    mode = batch-reset
  []
[]

[Transfers]
  [param_transfer]
    type = SamplerParameterTransfer
    to_multi_app = worker
    sampler = sample
    parameters = 'Kernels/source_u/value BCs/right_v/value'
  []
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
    type = MultiAppCloneReporterTransfer
    from_multi_app = worker
    from_reporters = 'jacobian_storage/indices'
    to_reporter = jac_indices
  []
[]

[Reporters]
  [parallel_storage]
    type = ParallelSolutionStorage
    variables = 'solution residual jacobian'
    outputs = out
  []
  [jac_indices]
    type = ConstantReporter
    outputs = out
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = none
  []
[]
