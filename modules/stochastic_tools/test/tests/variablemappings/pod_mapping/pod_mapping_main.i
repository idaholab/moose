[StochasticTools]
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
    execute_on = initial
    min_procs_per_row = 2
  []
[]

[MultiApps]
  [worker]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    mode = batch-restore
    min_procs_per_app = 2
  []
[]

[VariableMappings]
  [pod_mapping]
    type = PODMapping
    solution_storage = parallel_storage
    variables = "v"
    num_modes_to_compute = '2'
    extra_slepc_options = "-svd_monitor_all"
  []
[]

[Transfers]
  [param_transfer]
    type = SamplerParameterTransfer
    to_multi_app = worker
    sampler = sample
    parameters = 'Kernels/source_v/value BCs/right_v/value'
  []
  [solution_transfer]
    type = SerializedSolutionTransfer
    parallel_storage = parallel_storage
    from_multi_app = worker
    sampler = sample
    solution_container = solution_storage
    variables = 'v'
    serialize_on_root = true
  []
[]

[Reporters]
  [parallel_storage]
    type = ParallelSolutionStorage
    variables = 'v'
    outputs = none
  []
  [svd_output]
    type = SingularTripletReporter
    variables = 'v'
    pod_mapping = pod_mapping
    execute_on = FINAL
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = NONE
  []
[]
