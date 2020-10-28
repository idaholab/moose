[StochasticTools]
[]

[FormFunction]
  type = QuadraticMinimize
  initial_condition = '5 8 1'
  optimization_results = results
  objective = 1.0
  solution = '1 2 3'
[]

[Executioner]
  type = Optimize
  petsc_options_iname = '-tao_ntr_min_radius -tao_ntr_max_radius -tao_ntr_init_type'
  petsc_options_value = '0 1e16 constant'
  solve_on = none
[]

[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = sub.i
  []
[]

#transfer for a controllable variable
#list of postprocessors that I want to calculate for on the subapp in a transfer
# and then ultimately this would be an action.  corresponding vector for the target values for the post processors
[Transfers]
#full solution transfer  or use the postprocessor transfers but we should let the subapp aggregate the data into a quantity we can num_steps


  [send]
    type = MultiAppVppToVppTransfer
    vector_postprocessor_master = csv_reader_master
    vector_postprocessor_sub = receive_values_sub
    direction = to_multiapp
    multi_app = sub
    execute_on = timestep_begin
  []
  [receive]
    type = MultiAppVppToVppTransfer
    vector_postprocessor_sub = sample_points_sub
    vector_postprocessor_master = receive_values_master
    direction = from_multiapp
    multi_app = sub
    execute_on = timestep_end
  []
[]

[VectorPostprocessors]
  [csv_reader_master]
    type = CSVReader
    csv_file = 'input_master.csv'
    execute_on = 'INITIAL'
    force_preic = true
  []
  [receive_values_master]
    type = VectorPostprocessorReceiver
    execute_on = timestep_end
  []

  [data_pts]
    type = OptimizationResults

    #outputs=none
  []

[]

[Outputs]
  console = true
  file_base = 'zmaster/master'
  csv = true
[]
