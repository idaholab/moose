[StochasticTools]
[]

[Distributions]
  [S_dist]
    type = Uniform
    lower_bound = 2.5
    upper_bound = 7.5
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 4
    distributions = 'S_dist'
    execute_on = initial
    min_procs_per_row = 1
  []
[]

[MultiApps]
  [worker]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    mode = batch-restore
    min_procs_per_app = 1
  []
[]

[Mappings]
  [pod_mapping]
    type = PODMapping
    solution_storage = parallel_storage
  []
[]

[Transfers]
  [param_transfer]
    type = SamplerParameterTransfer
    to_multi_app = worker
    sampler = sample
    parameters = 'Kernels/source_u/value'
  []
  [solution_transfer]
    type = SerializedSolutionTransfer
    parallel_storage_name = parallel_storage
    from_multi_app = worker
    sampler = sample
    serialized_solution_reporter = solution_storage
    variables = 'u'
  []
[]

[Reporters]
  [parallel_storage]
    type = ParallelSolutionStorage
  []
  [reduced_solutions]
    type = MappingReporter
    sampler = sample
    parallel_storage = parallel_storage
    mapping = pod_mapping
    variables = "u"
  []
[]

[Executioner]
  type = Steady
  petsc_options = '-svd_monitor'
[]

# [Outputs]
#   [out]
#     type = CSV
#     execute_on = FINAL
#   []
# []
