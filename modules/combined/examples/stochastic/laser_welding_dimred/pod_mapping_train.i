[StochasticTools]
[]

[Distributions]
  [R_dist]
    type = Uniform
    lower_bound = 1.7257418583505537e-4
    upper_bound = 1.9257418583505537e-4
  []
  [power_dist]
    type = Uniform
    lower_bound = 170
    upper_bound = 190
  []
  [sb_dist]
    type = Uniform
    lower_bound = 5.17e-8
    upper_bound = 6.17e-8
  []
[]

[Samplers]
  [sample]
    type = LatinHypercube
    num_rows = 2
    distributions = 'R_dist power_dist sb_dist'
    min_procs_per_row = 2
    max_procs_per_row = 2
    execute_on = PRE_MULTIAPP_SETUP
  []
[]

[MultiApps]
  [worker]
    type = SamplerFullSolveMultiApp
    input_files = 2d.i
    sampler = sample
    mode = batch-reset
    min_procs_per_app = 2
    max_procs_per_app = 2
  []
[]

[VariableMappings]
  [pod_mapping_sol]
    type = PODMapping
    solution_storage = parallel_storage_sol
    variables = "T disp_x disp_y"
    num_modes_to_compute = '1 1 1'
    extra_slepc_options = "-svd_monitor_all"
  []
  # [pod_mapping_aux]
  #   type = PODMapping
  #   solution_storage = parallel_storage_aux
  #   variables = "vel_x_aux vel_y_aux"
  #   num_modes_to_compute = '20 20'
  #   extra_slepc_options = "-svd_monitor_all"
  # []
[]

[Controls]
  [cmdline]
    type = MultiAppSamplerControl
    multi_app = worker
    sampler = sample
    param_names = 'R power sb'
  []
[]

[Transfers]
  [solution_transfer_sol]
    type = SerializedSolutionTransfer
    parallel_storage = parallel_storage_sol
    from_multi_app = worker
    sampler = sample
    solution_container = solution_storage_sol
    variables = "T disp_x disp_y"
    serialize_on_root = false
  []
  # [solution_transfer_aux]
  #   type = SerializedSolutionTransfer
  #   parallel_storage = parallel_storage_aux
  #   from_multi_app = worker
  #   sampler = sample
  #   solution_container = solution_storage_aux
  #   variables = 'vel_x_aux vel_y_aux'
  #   serialize_on_root = true
  # []
[]

[Reporters]
  [parallel_storage_sol]
    type = ParallelSolutionStorage
    variables = "T disp_x disp_y"
    outputs = none
  []
  # [parallel_storage_aux]
  #   type = ParallelSolutionStorage
  #   variables = 'vel_x_aux vel_y_aux'
  #   outputs = none
  # []
  # [svd_output_sol]
  #   type = SingularTripletReporter
  #   variables = "T disp_x disp_y"
  #   pod_mapping = pod_mapping_sol
  #   execute_on = FINAL
  # []
  # [svd_output_aux]
  #   type = SingularTripletReporter
  #   variables = 'vel_x_aux vel_y_aux'
  #   pod_mapping = pod_mapping_aux
  #   execute_on = FINAL
  # []
  [reduced_sol]
    type = MappingReporter
    sampler = sample
    parallel_storage = parallel_storage_sol
    mapping = pod_mapping_sol
    variables = "T disp_x disp_y"
    execute_on = timestep_end
  []
  # [reduced_aux]
  #   type = MappingReporter
  #   sampler = sample
  #   parallel_storage = parallel_storage_aux
  #   mapping = pod_mapping_aux
  #   variables = 'vel_x_aux vel_y_aux'
  #   execute_on = timestep_end
  # []
  [matrix]
    type = StochasticMatrix
    sampler = sample
    parallel_type = ROOT
  []
[]

[Outputs]
  [out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = NONE
  []
  [mapping_sol]
    type = MappingOutput
    mappings = pod_mapping_sol
    execute_on = FINAL
  []
  # [mapping_aux]
  #   type = MappingOutput
  #   mappings = pod_mapping_aux
  #   execute_on = FINAL
  # []
[]
