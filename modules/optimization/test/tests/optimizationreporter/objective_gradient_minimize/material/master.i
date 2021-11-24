[StochasticTools]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 20
  ny = 20
  xmax = 2
  ymax = 2
[]

[OptimizationReporter]
  type = ObjectiveGradientMinimize
  parameter_names = 'p1'
  num_values = '1'
  initial_condition = '7'
  lower_bounds = '0'
  upper_bounds = '10'
  measurement_points = '0.2 0.2 0
            0.8 0.6 0
            0.2 1.4 0
            0.8 1.8 0'
  measurement_values = '226 254 214 146'
[]

[Executioner]
  type = Optimize
  tao_solver = taoblmvm
  petsc_options_iname = '-tao_gatol'
  petsc_options_value = '1e-4'
  # petsc_options_iname = '-tao_fd_gradient -tao_fd_delta -tao_gatol'
  # petsc_options_value = 'true 0.0001 1e-4'
  # petsc_options_iname='-tao_max_it -tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_fd_delta -tao_gatol -tao_gttol'
  # petsc_options_value='100 true true false 0.0001 0.0001 0.000001'
  verbose = true
[]

[MultiApps]
  [forward]
    type = OptimizeFullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
    clone_master_mesh = true
    ignore_solve_not_converge = false
  []
  [adjoint]
    type = OptimizeFullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
    clone_master_mesh = true
    ignore_solve_not_converge = false
  []
[]

[AuxVariables]
  [temperature_forward]
    order = FIRST
    family = LAGRANGE
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
      from_reporters = 'OptimizationReporter/measurement_points OptimizationReporter/misfit_values'
      to_reporters = 'misfit/measurement_points misfit/misfit_values'
    []



  [to_forward]
    type = OptimizationParameterTransfer
    multi_app = forward
    value_names = 'p1'
    parameters = 'Postprocessors/p1/value'
    to_control = parameterReceiver
  []
  [from_forward_temp]
    type = MultiAppCopyTransfer
    multi_app = forward
    direction = from_multiapp
    source_variable = 'temperature'
    variable = 'temperature_forward'
  []

  [to_adjoint_param]
    type = OptimizationParameterTransfer
    multi_app = adjoint
    value_names = 'p1'
    parameters = 'Postprocessors/p1/value'
    to_control = parameterReceiver
  []
  [to_adjoint_temp]
    type = MultiAppCopyTransfer
    multi_app = adjoint
    direction = to_multiapp
    source_variable = 'temperature_forward'
    variable = 'temperature_forward'
  []

  [from_adjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    direction = from_multiapp
    from_reporters = 'adjoint_grad/adjoint_grad'
    to_reporters = 'OptimizationReporter/adjoint'
    # The to_reporters is always OptimizationReporter/adjoint' for the gradient
    #'OptimizationReporter/adjoint' is automatically created by ObjectiveGradientMinimize
  []
[]

[Reporters]
  [receiver]
    type = ConstantReporter
    real_vector_names = measured
    real_vector_values = '0'
   []
[]

[Outputs]
  file_base = 'master'
  console = false
  csv=true
  exodus = false
[]
