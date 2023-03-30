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
    execute_on = initial
    min_procs_per_row = 2
  []
  [sample_test]
    type = MonteCarlo
    num_rows = 100
    distributions = 'S_dist D_dist'
    execute_on = initial
    min_procs_per_row = 2
    seed = 11
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

[Trainers]
  [polyreg]
    type = PolynomialRegressionTrainer
    sampler = sample
    regression_type = ols
    # penalty = 0.1
    max_degree = 1
    response = reduced_solutions/v_pod_mapping
    response_type = vector_real
    execute_on = FINAL
  []
[]

[Surrogates]
  [polyreg]
    type = PolynomialRegressionSurrogate
    trainer = polyreg
  []
[]

[Mappings]
  [pod_mapping]
    type = PODMapping
    solution_storage = parallel_storage
    variables = "u v"
    num_modes = '10 10'
    extra_slepc_options = "-svd_monitor_all"
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
    type = SerializedSolutionTransfer
    parallel_storage_name = parallel_storage
    from_multi_app = worker
    sampler = sample
    serialized_solution_reporter = solution_storage
    variables = 'u v'
    serialize_on_root = false
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
    variables = "u v"
    execute_on = timestep_end
  []
  [eval]
    type = EvaluateSurrogate
    model = polyreg
    response_type = vector_real
    parallel_type = ROOT
    execute_on = FINAL
    sampler = sample_test
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
  []
  [rom]
    type = SurrogateTrainerOutput
    trainers = polyreg
    execute_on = FINAL
  []
[]
