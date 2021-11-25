[StochasticTools]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 20
  xmax = 1
  ymax = 2
  bias_x = 1.0
  bias_y = 1.0
[]

[AuxVariables]
  [temperature_forward]
    order = FIRST
    family = LAGRANGE
  []
[]

[OptimizationReporter]
  type = ObjectiveGradientMinimize
  parameter_names = 'p1'
  num_values = '1'
  initial_condition = '9'
  upper_bounds = '10'
  lower_bounds = '1'
  measurement_points = '0.1	0	0
                        0.1	0.1	0
                        0.1	0.2	0
                        0.1	0.3	0
                        0.1	0.4	0
                        0.1	0.5	0
                        0.1	0.6	0
                        0.1	0.7	0
                        0.1	0.8	0
                        0.1	0.9	0
                        0.1	1	0
                        0.1	1.1	0
                        0.1	1.2	0
                        0.1	1.3	0
                        0.1	1.4	0
                        0.1	1.5	0
                        0.1	1.6	0
                        0.1	1.7	0
                        0.1	1.8	0
                        0.1	1.9	0
                        0.1	2	0'
  measurement_values = '500
                        472.9398111
                        450.8117197
                        434.9560747
                        423.3061045
                        414.9454912
                        409.3219399
                        406.1027006
                        405.0865428
                        406.1604905
                        409.2772668
                        414.4449772
                        421.7253934
                        431.2401042
                        443.1862012
                        457.8664824
                        475.7450186
                        497.5582912
                        524.4966003
                        559.1876637
                        600'
[]

[Executioner]
  type = Optimize
  tao_solver = taoblmvm #taolmvm#taonm #taolmvm
  petsc_options_iname = '-tao_gatol' # -tao_fd_gradient -tao_fd_delta'
  petsc_options_value = '1e-4' #1e-1 '#true 1e-4'

#   petsc_options_iname='-tao_max_it -tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_fd_delta -tao_gatol'
#   petsc_options_value='1 true true false 1e-6 0.1'
#   petsc_options = '-tao_test_gradient_view'
   verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
    clone_master_mesh = true
  []
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
    clone_master_mesh = true
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
    type = OptimizationParameterTransfer
    multi_app = forward
    value_names = 'p1'
    parameters = 'Postprocessors/p1/value'
    to_control = parameterReceiver
  []
  [fromAdjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    from_reporters = 'adjoint_pt/adjoint_pt'
    to_reporters = 'OptimizationReporter/adjoint'
    direction = from_multiapp
  []

#these are transferring data from subapp to subapp because the adjoint problem
# needs the forward solution to compute the gradient.  Maybe this step could be
# done on the main app.  The adjoint only passes the adjoint variable (whole mesh)
# to the main app and the main app computes the gradient from this.
  [fromForward_temp]
    type = MultiAppCopyTransfer
    multi_app = forward
    direction = from_multiapp
    source_variable = 'temperature'
    variable = 'temperature_forward'
  []
  [toAdjoint_temp]
    type = MultiAppCopyTransfer
    multi_app = adjoint
    direction = to_multiapp
    source_variable = 'temperature_forward'
    variable = 'temperature_forward'
  []

  #This is to get the parameter used in the forward problem onto the adjoint problem
  [toAdjoint_param]
    type = OptimizationParameterTransfer
    multi_app = adjoint
    value_names = 'p1'
    parameters = 'Postprocessors/p1/value'
    to_control = adjointReceiver
  []

[]

[Reporters]
  [receiver]
    type = ConstantReporter
    real_vector_names = measured
    real_vector_values = '0 0 0 0'
  []
[]

[Outputs]
  csv = true
[]
