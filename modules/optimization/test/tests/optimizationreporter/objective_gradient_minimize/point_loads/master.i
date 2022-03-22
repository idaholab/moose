[StochasticTools]
[]

[OptimizationReporter]
  type = ObjectiveGradientMinimize
  parameter_names = 'parameter_results'
  num_values = '3'
  measurement_points = '0.3 0.3 0
            0.4 1.0 0
            0.8 0.5 0
            0.8 0.6 0'
  measurement_values = '100 204 320 216'
[]

# Page 146 of PetSc/Tao manual - start with Newton Linesearch
# Look at default values including the preconditioner
[Executioner]
  type = Optimize
  tao_solver = taobncg
  petsc_options_iname = '-tao_gatol -tao_max_it'
  petsc_options_value = '1e-1 50'
  verbose = false
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
  # the forward problem has homogeneous boundary conditions so it can be reused here.
  [homogeneousForward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "HOMOGENEOUS_FORWARD"
  []
[]

[Transfers]
#these are usually the same for all input files.
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'data_pt/temperature data_pt/temperature'
    to_reporters = 'OptimizationReporter/simulation_values receiver/measured'
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
#these are different,
# - to forward depends on teh parameter being changed
# - from adjoint depends on the gradient being computed from the adjoint
#NOTE:  the adjoint variable we are transferring is actually the gradient
  [toForward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/parameter_results'
    to_reporters = 'point_source/value'
  []
  [fromAdjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'data_pt/temperature'
    to_reporters = 'OptimizationReporter/adjoint'
  []

  # HESSIAN transfers.  Same as forward.
  [fromHomoForward]
    type = MultiAppReporterTransfer
    multi_app = homogeneousForward
    direction = from_multiapp
    from_reporters = 'data_pt/temperature data_pt/temperature'
    to_reporters = 'OptimizationReporter/simulation_values receiver/measured'
  []
  [toHomoForward_measument]
    type = MultiAppReporterTransfer
    multi_app = homogeneousForward
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord'
    to_reporters = 'measure_data/measurement_xcoord measure_data/measurement_ycoord measure_data/measurement_zcoord'
  []
  [toHomoForward]
    type = MultiAppReporterTransfer
    multi_app = homogeneousForward
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/parameter_results'
    to_reporters = 'point_source/value'
  []
[]

[Reporters]
  [receiver]
    type = ConstantReporter
    real_vector_names = measured
    real_vector_values = '0 0 0 0'
  []
  [optInfo]
    type = OptimizationInfo
    #items = 'current_iterate'
    #execute_on=timestep_end
  []
[]

[Outputs]
  console = true
  csv=true
  json=true
[]
