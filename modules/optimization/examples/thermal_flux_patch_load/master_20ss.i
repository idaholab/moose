[StochasticTools]
[]

[OptimizationReporter]
  type = ObjectiveGradientMinimize
  parameter_names = 'p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12 p13 p14 p15 p16 p17 p18 p19'
  num_values = '1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1 1'
  measurement_points = '4.24  0  2.45
                        4.24  2.45  0
                        4.24  0  -2.45
                        4.24  -2.45  0
                        2.45  0  4.24
                        2.45  4.24  0
                        2.45  0  -4.24
                        2.45  -4.24  0
                        4.9 0 0'
  measurement_values = '296.8965126
                        299.1226708
                        296.9036375
                        294.6646469
                        299.909442
                        305.0568193
                        299.9158336
                        294.7690098
                        295.9769139'
[]

[Executioner]
  type = Optimize
  tao_solver = taonm
  petsc_options_iname = '-tao_gatol'# -tao_cg_delta_max'
  petsc_options_value = '1e-2'
  verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward_20ss.i
    execute_on = "FORWARD"
  []
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint_20ss.i
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
[toforward]
  type = OptimizationParameterTransfer
  multi_app = forward
  value_names = 'p0 p1 p2 p3 p4 p5 p6 p7 p8 p9 p10 p11 p12 p13 p14 p15 p16 p17 p18 p19'
  parameters = 'Postprocessors/p0/value
                Postprocessors/p1/value
                Postprocessors/p2/value
                Postprocessors/p3/value
                Postprocessors/p4/value
                Postprocessors/p5/value
                Postprocessors/p6/value
                Postprocessors/p7/value
                Postprocessors/p8/value
                Postprocessors/p9/value
                Postprocessors/p0/value
                Postprocessors/p1/value
                Postprocessors/p2/value
                Postprocessors/p3/value
                Postprocessors/p4/value
                Postprocessors/p5/value
                Postprocessors/p6/value
                Postprocessors/p7/value
                Postprocessors/p8/value
                Postprocessors/p9/value'
  to_control = parameterReceiver
[]
  [fromadjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    direction = from_multiapp
    from_reporters = 'adjoint_bc/adjoint_bc' # what is the naming convention for this
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
