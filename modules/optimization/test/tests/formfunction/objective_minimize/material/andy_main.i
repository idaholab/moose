# This main file runs the subapp andy_model.i
# The single parameter to be found by inversion is diffusivity_parameter.  Using an OptimizationParameterTransfer, this is transferred to the diffusivity_postprocessor value in andy_model.i.  It is initialized to 0.8, which is quite close to the solution of 1.0
# This file passes the diffusivity to andy_model.i via the OptimizationParameterTransfer defined below
# This file retrieves from andy_model.i the misfit of the temperature predicted by andy_model.i and the desired temperature.  This occurs through the MultiAppReporterTransfer defined below
# The PETSc-TAO lmvm solver is used.  This forms finite-difference approximations of the Jacobian by running andy_model.i with slightly different diffusivity values, and then hopefully converges to the solution.
[StochasticTools]
[]

[FormFunction]
  type = ObjectiveMinimize
  parameter_names = diffusivity_parameter
  num_values = 1
  initial_condition = 0.8
  misfit_name = misfit
[]

[Executioner]
  type = Optimize
  tao_solver = taolmvm
#  petsc_options = '-tao_ls_view'
#   petsc_options_iname = '-tao_max_it -tao_max_funcs -tao_ls_max_funcs -tao_ls_ftol -tao_ls_rtol -tao_fd_gradient -tao_fd_delta -tao_gatol -tao_ls_type'
#   petsc_options_value = '10           31             310               1E-2         1E-1         true             1E-5          1E-4       more-thuente'
   petsc_options_iname = '-tao_fd_gradient -tao_gatol'
   petsc_options_value = ' true            1E-6'
  verbose = true
[]

[MultiApps]
  [andy_model]
    type = OptimizeFullSolveMultiApp
    reset_app = true # this is necessary in order that andy_model.i starts from temperature=0 each time it is called
    input_files = andy_model.i
    execute_on = FORWARD
  []
[]

[Transfers]
  [diffusivity_to_andy_model]
    type = OptimizationParameterTransfer
    multi_app = andy_model
    to_control = diffusivityReceiver
    value_names = diffusivity_parameter
    parameters = 'Postprocessors/diffusivity_postprocessor/value'
  []

  [from_andy_model]
    type = MultiAppReporterTransfer
    multi_app = andy_model
    direction = from_multiapp
    from_reporters = 'measuring_point/temperature_difference measuring_point/temperature'
    to_reporters = 'FormFunction/misfit receiver_t/simulated_temperature'
  []
  [diffusivity_from_andy_model]
    type = MultiAppPostprocessorTransfer
    multi_app = andy_model
    direction = from_multiapp
    reduction_type = average
    from_postprocessor = diffusivity_postprocessor
    to_postprocessor = diffusivity
  []
[]

[Postprocessors]
  [diffusivity]
    type = ConstantValuePostprocessor
    value = 0.8
  []
[]		
		
[Reporters]
  [receiver_t]
    type = ConstantReporter
    real_vector_names = simulated_temperature
    real_vector_values = 0 # dummy initial value
  []
  [optInfo]
    type=OptimizationInfo
  []
[]

[Outputs]
  console = true
  csv=true
[]
