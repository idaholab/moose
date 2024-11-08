[StochasticTools]
[]

[Distributions]
  [R_dist]
    type = Uniform
    lower_bound = 1.25E-4
    upper_bound = 1.55E-4
  []
  [power_dist]
    type = Uniform
    lower_bound = 60
    upper_bound = 74
  []
[]

[Samplers]
  [train]
    type = MonteCarlo
    num_rows = 45
    distributions = 'R_dist power_dist'
    execute_on = PRE_MULTIAPP_SETUP
    min_procs_per_row = 2
    max_procs_per_row = 2
  []
[]

[MultiApps]
  [worker]
    type = SamplerFullSolveMultiApp
    input_files = 2d.i
    sampler = train
    mode = batch-reset
    min_procs_per_app = 2
    max_procs_per_app = 2
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = worker
    sampler = train
    param_names = 'R power'
  []
[]

[Transfers]
  [solution_transfer]
    type = SerializedSolutionTransfer
    parallel_storage = parallel_storage
    from_multi_app = worker
    sampler = train
    solution_container = solution_storage
    variables = "T"
    serialize_on_root = true
  []
[]

[Reporters]
  [parallel_storage]
    type = ParallelSolutionStorage
    variables = "T"
    outputs = none
  []
  [reduced_sol]
    type = MappingReporter
    sampler = train
    parallel_storage = parallel_storage
    mapping = pod
    variables = "T"
    outputs = json
    execute_on = final
    parallel_type = ROOT
  []
  [matrix]
    type = StochasticMatrix
    sampler = train
    parallel_type = ROOT
  []
  [svd]
    type = SingularTripletReporter
    pod_mapping = pod
    variables = "T"
    execute_on = final
  []
[]

[VariableMappings]
  [pod]
    type = PODMapping
    solution_storage = parallel_storage
    variables = "T"
    num_modes_to_compute = '8'
    extra_slepc_options = "-svd_monitor_all"
  []
[]

[Trainers]
  [mogp]
    type = GaussianProcessTrainer
    execute_on = final
    covariance_function = 'lmc'
    standardize_params = 'true'
    standardize_data = 'true'
    sampler = train
    response_type = vector_real
    response = reduced_sol/T_pod
    tune_parameters = 'lmc:acoeff_0 lmc:lambda_0 covar:signal_variance covar:length_factor'
    tuning_min = '1e-9 1e-9 1e-9 1e-9'
    tuning_max = '1e16 1e16 1e16  1e16'
    num_iters = 10000
    learning_rate = 0.0005
    show_every_nth_iteration = 10
  []
[]

[Covariance]
  [covar]
    type = SquaredExponentialCovariance
    signal_variance = 1.0
    noise_variance = 0.0
    length_factor = '0.1 0.1'
  []
  [lmc]
    type = LMC
    covariance_functions = covar
    num_outputs = 8
    num_latent_funcs = 1
  []
[]


[Outputs]
  [json]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = NONE
  []
  [pod_out]
    type = MappingOutput
    mappings = pod
    execute_on = FINAL
  []
  [mogp_out]
    type = SurrogateTrainerOutput
    trainers = mogp
    execute_on = FINAL
  []
[]
