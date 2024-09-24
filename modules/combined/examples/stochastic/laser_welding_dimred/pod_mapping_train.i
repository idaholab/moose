[StochasticTools]
[]

[Distributions]
  [R_dist]
    type = Uniform
    lower_bound = 1.25e-4
    upper_bound = 1.7e-4
  []
  [power_dist]
    type = Uniform
    lower_bound = 70
    upper_bound = 83
  []
[]

[Samplers]
  [sample]
    type = MonteCarlo
    num_rows = 120
    distributions = 'R_dist power_dist'
    min_procs_per_row = 56
    max_procs_per_row = 56
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [worker]
    type = SamplerFullSolveMultiApp
    input_files = 2d.i
    sampler = sample
    mode = batch-reset
    min_procs_per_app = 56
    max_procs_per_app = 56
  []
[]

[VariableMappings]
  [pod_mapping_sol]
    type = PODMapping
    solution_storage = parallel_storage_sol
    variables = "T"
    num_modes_to_compute = '10'
    extra_slepc_options = "-svd_monitor_all"
  []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = worker
    sampler = sample
    param_names = 'R power'
  []
[]

[Transfers]
  [solution_transfer_sol]
    type = SerializedSolutionTransfer
    parallel_storage = parallel_storage_sol
    from_multi_app = worker
    sampler = sample
    solution_container = solution_storage_sol
    variables = "T"
    serialize_on_root = true
  []
  [data]
    type = SamplerReporterTransfer
    from_multi_app = worker
    sampler = sample
    from_reporter = 'tot_nl/value'
    stochastic_reporter = 'matrix'
  []
[]

[Reporters]
  [parallel_storage_sol]
    type = ParallelSolutionStorage
    variables = "T"
    outputs = none
  []
  [svd_output_sol]
    type = SingularTripletReporter
    variables = "T"
    pod_mapping = pod_mapping_sol
    execute_on = FINAL
  []
  [reduced_sol]
    type = MappingReporter
    sampler = sample
    parallel_storage = parallel_storage_sol
    mapping = pod_mapping_sol
    variables = "T"
    execute_on = timestep_end
    parallel_type = ROOT
  []
  [matrix]
    type = StochasticMatrix
    sampler = sample
    parallel_type = ROOT
  []
[]

[Trainers]
  [mogp_trainer]
    type = GaussianProcessTrainer
    execute_on = timestep_end
    covariance_function = 'lmc'
    standardize_params = 'true'
    standardize_data = 'true'
    sampler = sample
    response_type = vector_real
    response = reduced_sol/T_pod_mapping_sol
    tune_parameters = 'lmc:acoeff_0 lmc:lambda_0 covar:signal_variance covar:length_factor'
    tuning_min = '1e-9 1e-9 1e-9 1e-9'
    tuning_max = '1e16 1e16 1e16  1e16'
    num_iters = 1000
    batch_size = 120
    learning_rate = 0.001
    show_every_nth_iteration = 10
  []
[]

[Covariance]
  [covar]
    type = SquaredExponentialCovariance
    signal_variance = 1.0
    noise_variance = 0.0
    length_factor = '0.01 0.01'
  []
  [lmc]
    type = LMC
    covariance_functions = covar
    num_outputs = 10
    num_latent_funcs = 1
  []
[]

[Surrogates]
  [mogp]
    type = GaussianProcessSurrogate
    trainer = mogp_trainer
  []
[]

[VectorPostprocessors]
  [hyperparams]
    type = GaussianProcessData
    gp_name = mogp
    execute_on = final
    outputs = out
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = NONE
    vectorpostprocessors_as_reporters = true
  []
  [mapping_sol]
    type = MappingOutput
    mappings = pod_mapping_sol
    execute_on = FINAL
  []
  [surr]
    type = SurrogateTrainerOutput
    execute_on = FINAL
    trainers = "mogp_trainer"
  []
[]
