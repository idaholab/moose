[StochasticTools]
[]

[OptimizationReporter]
  type = ObjectiveGradientMinimize
  parameter_names = 'parameter_results'
  num_values = '5'
  measurement_points = '4.24	0	2.45
   4.24	2.45	0
   4.24	0	-2.45
   4.24	-2.45	0
   2.45	0	4.24
   2.45	4.24	0
   2.45	0	-4.24
   2.45	-4.24	0
   4.9 0 0'
  measurement_values = '221.5542848     220.3392602     221.1322675     222.3562443     220.9615351     219.2140933     220.3868245     222.1432008     222.9280329'
[]

[Executioner]
  type = Optimize
  tao_solver = taolmvm
  petsc_options_iname = '-tao_gatol'# -tao_cg_delta_max'
  petsc_options_value = '1e-2'
  verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
  []
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
  []
[]

[Transfers]
  #these are usually the same for all input files.
    [fromForward]
      type = MultiAppReporterTransfer
      multi_app = forward
      direction = from_multiapp
      from_reporters = 'data_pt/temperature data_pt/temperature'
      to_reporters = 'OptimizationReporter/simulation_values receiver/measured'
    []
    [toAdjoint]
      type = MultiAppReporterTransfer
      multi_app = adjoint
      direction = to_multiapp
      from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord OptimizationReporter/misfit_values'
      to_reporters = 'misfit/measurement_xcoord misfit/measurement_ycoord misfit/measurement_zcoord misfit/misfit_values'
    []
    [toForward_measument]
      type = MultiAppReporterTransfer
      multi_app = forward
      direction = to_multiapp
      from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord'
      to_reporters = 'measure_data/measurement_xcoord measure_data/measurement_ycoord measure_data/measurement_zcoord'
    []

#these are different,
# - to forward depends on teh parameter being changed
# - from adjoint depends on the gradient being computed from the adjoint
#NOTE:  the adjoint variable we are transferring is actually the gradient
  [toForward]
    type = MultiAppReporterTransfer
    multi_app = forward
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/parameter_results'
    to_reporters = 'point_source/value'
  []
  [fromAdjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    direction = from_multiapp
    from_reporters = 'data_pt/temperature'
    to_reporters = 'OptimizationReporter/adjoint'
  []
[]

[Reporters]
  [receiver]
    type = ConstantReporter
    real_vector_names = measured
    real_vector_values = '0'
  []
  [optInfo]
    type = OptimizationInfo
  []
[]

[Outputs]
  csv=true
[]
