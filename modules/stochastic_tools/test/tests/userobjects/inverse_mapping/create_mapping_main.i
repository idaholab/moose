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
    num_rows = 8
    distributions = 'S_dist D_dist'
    execute_on = PRE_MULTIAPP_SETUP
    min_procs_per_row = 2
  []
[]

[MultiApps]
  [worker]
    type = SamplerFullSolveMultiApp
    input_files = sub.i
    sampler = sample
    mode = batch-reset
    min_procs_per_app = 2
  []
[]

[Trainers]
  [polyreg_v]
    type = PolynomialRegressionTrainer
    sampler = sample
    regression_type = ols
    max_degree = 1
    response = reduced_solutions/v_pod_mapping
    response_type = vector_real
    execute_on = FINAL
  []
  [polyreg_v_aux]
    type = PolynomialRegressionTrainer
    sampler = sample
    regression_type = ols
    max_degree = 1
    response = reduced_solutions/v_aux_pod_mapping
    response_type = vector_real
    execute_on = FINAL
  []
[]

[VariableMappings]
  [pod_mapping]
    type = PODMapping
    solution_storage = parallel_storage
    variables = "v v_aux"
    num_modes_to_compute = '8 8'
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
    serialize_on_root = false
  []
  [solution_transfer_aux]
    type = SerializedSolutionTransfer
    parallel_storage = parallel_storage
    from_multi_app = worker
    sampler = sample
    solution_container = solution_storage_aux
    variables = 'v_aux'
    serialize_on_root = false
  []
[]

[Controls]
  [cmd_line]
    type = MultiAppSamplerControl
    multi_app = worker
    sampler = sample
    param_names = 'S D'
  []
[]

[Reporters]
  [parallel_storage]
    type = ParallelSolutionStorage
    variables = 'v v_aux'
    outputs = none
  []
  [reduced_solutions]
    type = MappingReporter
    sampler = sample
    parallel_storage = parallel_storage
    mapping = pod_mapping
    variables = "v v_aux"
    execute_on = timestep_end # To make sure the trainer sees the results on FINAL
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
  []
  [mapping]
    type = MappingOutput
    mappings = pod_mapping
    execute_on = FINAL
  []
  [rom]
    type = SurrogateTrainerOutput
    trainers = 'polyreg_v polyreg_v_aux'
    execute_on = FINAL
  []
[]
