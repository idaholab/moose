# This tests optimization of the function a*sin(2*pi*b*(y+c))+d
# where c and d are being parameterized for.  This function is
# also being applied to the top, left, and bottom surfaces.
# This is nonlinear since the gradient term contains the parameters,
# we pass the parameters into the adjoint function because they show up
# in the derivative of the sin function that is integrated over the boundaries.
# Optimization is able to find a set of parameters that match the measurement
# points but they are not unique and the phase of the function is
# all over the place.


[StochasticTools]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 20
  xmax = 1
  ymax = 2
  bias_x = 1.05
  bias_y = 1.05
[]


[OptimizationReporter]
  type = ObjectiveGradientMinimize
  parameter_names = 'p1 p2'
  num_values = '1 1'
  misfit_name = 'misfit'
  adjoint_data_name = 'adjoint'
  initial_condition = '0 500'
  upper_bounds = '2 1500'
  lower_bounds = '0 0'
  # measured data for p1=1, p2=1000
  # not sure if bounds are used when -tao_ls_type=unit
[]

[Executioner]
  type = Optimize
  tao_solver = taoblmvm
  petsc_options_iname = '-tao_gatol -tao_ls_type'
  petsc_options_value = '1e-4 unit'
  # petsc_options_iname = '-tao_fd_gradient -tao_fd_delta -tao_gatol'
  # petsc_options_value = 'true 0.0001 1e-4'
   # petsc_options = '-tao_test_gradient_view'
   # petsc_options_iname='-tao_max_it -tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_fd_delta -tao_gatol'
   # petsc_options_value='1 true true false 0.0001 0.0001'
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
    value_names = 'p1 p2'
    parameters = 'Postprocessors/p1/value Postprocessors/p2/value'
    to_control = parameterReceiver
  []
  [fromforward]
    type = MultiAppReporterTransfer
    multi_app = forward
    direction = from_multiapp
    from_reporters = 'data_pt/temperature_difference data_pt/temperature'
    to_reporters = 'OptimizationReporter/misfit receiver/measured'
  []
  [toadjoint_params]
    type = OptimizationParameterTransfer
    multi_app = adjoint
    value_names = 'p1'
    parameters = 'Postprocessors/p1/value'
    to_control = adjointReceiver
  []
  [toadjoint_misfit]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/misfit'
    to_reporters = 'point_source/value'
  []
  [fromadjoint]
    type = MultiAppReporterTransfer
    multi_app = adjoint
    direction = from_multiapp
    from_reporters = 'adjoint_bc/adjoint_bc'
    to_reporters = 'OptimizationReporter/adjoint'
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
