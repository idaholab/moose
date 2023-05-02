# This main.i file runs the subapps model.i and grad.i, using an FullSolveMultiApp
# The purpose of main.i is to find the two diffusivity_values
# (one in the bottom material of model.i, and one in the top material of model.i)
# such that the misfit between experimental observations (defined in model.i) and MOOSE predictions is minimised.
# The adjoint computed in grad.i is used to compute the gradient for the gradient based LMVM solver in TAO
# PETSc-TAO optimisation is used to perform this inversion
#
[Optimization]
[]

[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = diffusivity_values
  num_values = 2 # diffusivity in the bottom material and in the top material of model.i
  initial_condition = '3 4' # the expected result is about '1 10' so this initial condition is not too bad
  lower_bounds = '1'
  upper_bounds = '20'
  measurement_file = 'synthetic_data.csv'
  file_value = 'temperature'
[]

[Executioner]
  type = Optimize
  tao_solver = taoblmvm
  petsc_options_iname = '-tao_gatol'
  petsc_options_value = '1e-3'
  ## THESE OPTIONS ARE FOR TESTING THE ADJOINT GRADIENT
  # petsc_options_iname='-tao_max_it -tao_fd_test -tao_test_gradient -tao_fd_gradient -tao_fd_delta -tao_gatol'
  # petsc_options_value='1 true true false 1e-8 0.1'
  # petsc_options = '-tao_test_gradient_view'
  verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = model_and_adjoint.i
    execute_on = FORWARD
  []
[]

[Transfers]
  [toForward] #pass the coordinates where we knew the measurements to the forward model to do the extraction of the simulation data at the location of the measurements to compute the misfit
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/measurement_values
                      OptimizationReporter/diffusivity_values'
    to_reporters = 'measure_data/measurement_xcoord
                    measure_data/measurement_ycoord
                    measure_data/measurement_zcoord
                    measure_data/measurement_time
                    measure_data/measurement_values
                    data/diffusivity'
  []
  [from_forward] #get the simulation values
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measure_data/simulation_values
                      gradvec/inner_product'
    to_reporters = 'OptimizationReporter/simulation_values
                    OptimizationReporter/grad_diffusivity_values'
  []
[]

[Outputs]
  console = true
  csv = true
[]
