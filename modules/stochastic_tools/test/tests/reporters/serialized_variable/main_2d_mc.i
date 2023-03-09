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
    to_multi_app = worker
    sampler = sample
  []
[]

[Reporters]
  [parallel_storage]
    type = ParallelSolutionStorage
  []
[]

# [Outputs]
#   [out]
#     type = CSV
#     execute_on = FINAL
#   []
# []
