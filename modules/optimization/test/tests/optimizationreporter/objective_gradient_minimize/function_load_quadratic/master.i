[StochasticTools]
[]

[OptimizationReporter]
  type = ObjectiveGradientMinimize
  parameter_names = 'parameter_results'
  num_values = '3'
  initial_condition = '1 1 1'
  measurement_points = '0.2 0.5 0
            0.5 0.5 0
            1.5 0.5 0
            1.8 0.5 0
            0.3 1.1 0
            0.6 1.1 0
            0.9 1.1 0
            1.2 1.1 0'
  measurement_values = '209 221 159 121 260 271 265 236'
[]

[Executioner]
  type = Optimize
  tao_solver =taolmvm
  petsc_options_iname = '-tao_max_it -tao_fmin -tao_ls_type' # tao_gatol'
  petsc_options_value = '50 4 unit' #1e-4
   # petsc_options_iname='-tao_max_it -tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_fd_delta -tao_gatol'
   # petsc_options_value='1 true true false 0.0001 0.0001'
  verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = FORWARD
    reset_app = true
  []
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint.i
    execute_on = ADJOINT
    reset_app = true
  []
[]

[Controls]
  [toforward]
    type = OptimizationMultiAppCommandLineControl
    multi_app = forward
    value_names = 'parameter_results'
    parameters = 'Functions/volumetric_heat_func/vals'
  []
  [toadjoint]
    type = OptimizationMultiAppCommandLineControl
    multi_app = adjoint
    value_names = 'parameter_results'
    parameters = 'function_vals'
  []
[]

[Transfers]
  #these are usually the same for all input files.
    [fromForward]
      type = MultiAppReporterTransfer
      from_multi_app = forward
      from_reporters = 'data_pt/temperature'
      to_reporters = 'OptimizationReporter/simulation_values'
    []
    [toAdjoint]
      type = MultiAppReporterTransfer
      to_multi_app = adjoint
      from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord OptimizationReporter/misfit_values'
      to_reporters = 'misfit/measurement_xcoord misfit/measurement_ycoord misfit/measurement_zcoord misfit/misfit_values'
    []
    [toForward_measument]
      type = MultiAppReporterTransfer
      to_multi_app = forward
      from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord'
      to_reporters = 'measure_data/measurement_xcoord measure_data/measurement_ycoord measure_data/measurement_zcoord'
    []
  [fromadjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'adjoint_pt/adjoint_pt'
    to_reporters = 'OptimizationReporter/adjoint'
  []
[]

[Reporters]
  [optInfo]
    type = OptimizationInfo
  []
[]

[Outputs]
  # console = true
  csv=true
[]
