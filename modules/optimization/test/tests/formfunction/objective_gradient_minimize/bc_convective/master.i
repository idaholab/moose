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

[FormFunction]
  type = ObjectiveGradientMinimize
  parameter_names = 'p1'
  num_values = '1'
  misfit_name = 'misfit'
  adjoint_data_name = 'adjoint'
  initial_condition = '9'
  upper_bounds = '10'
  lower_bounds = '1'
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
  [toforward]
    type = OptimizationParameterTransfer
    multi_app = forward
    value_names = 'p1'
    parameters = 'Postprocessors/p1/value'
    to_control = parameterReceiver
  []
  [fromforward]
    type = MultiAppReporterTransfer
    multi_app = forward
    from_reporters = 'data_pt/temperature_difference data_pt/temperature'
    to_reporters = 'FormFunction/misfit receiver/measured'
    direction = from_multiapp
  []
  [from_forward_temp]
    type = MultiAppCopyTransfer
    multi_app = forward
    direction = from_multiapp
    source_variable = 'temperature'
    variable = 'temperature_forward'
  []

  [toadjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    from_reporters = 'FormFunction/misfit'
    to_reporters = 'point_source/value'
    direction = to_multiapp
  []
  [toadjoint_param]
    type = OptimizationParameterTransfer
    multi_app = adjoint
    value_names = 'p1'
    parameters = 'Postprocessors/p1/value'
    to_control = adjointReceiver
  []
  [to_adjoint_temp]
    type = MultiAppCopyTransfer
    multi_app = adjoint
    direction = to_multiapp
    source_variable = 'temperature_forward'
    variable = 'temperature_forward'
  []

  [fromadjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    from_reporters = 'adjoint_pt/adjoint_pt'
    to_reporters = 'FormFunction/adjoint'
    direction = from_multiapp
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
