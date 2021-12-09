# This main.i file runs the subapp model.i, using an OptimizeFullSolveMultiApp
# The purpose of main.i is to find the two diffusivity_values (one in the bottom material of model.i, and one in the top material of model.i) such that the misfit between experimental observations (defined in model.i) and MOOSE predictions is minimised.
# PETSc-TAO optimisation is used to perform this inversion
#
# PETSc-TAO is set to uses "lmvm" and finite-difference approximations to the derivative of the objective function
# This means that for each PETSc-TAO iteration, 5 runs of model.i are performed: one for the current value of diffusivity_values, and four more for forward and backward finite differences around this point.  (Hence, if there were n diffusivity values, PETSc-TAO would run model.i 1 + 2*n times per iteration).
#
# This file passes the diffusivity_values to model.i via the MultiAppReporterTransfer defined below
# This file retrieves from model.i: the misfit of the temperature predicted by model.i and the experimental observations; the temperature at the observation points; the diffusivity_values (so they can be recorded in the output).  This occurs through the MultiAppReporterTransfer defined below
[StochasticTools]
[]

[OptimizationReporter]
  type = ObjectiveMinimize
  parameter_names = diffusivity_values
  num_values = 2 # diffusivity in the bottom material and in the top material of model.i
  initial_condition = '2.0 2.0' # the expected result is about '1 10' so this initial condition is not too bad
  lower_bounds = '0 0'
  upper_bounds = '20 20'
  measurement_points = '-2 -2 0
             0 -2 0
             2 -2 0
             0 2 0'
  measurement_values = '0.022 0.040 0.022 0.137'
[]

[Executioner]
  type = Optimize
  tao_solver = taolmvm
  petsc_options_iname = '-tao_fd_gradient -tao_gatol'
  petsc_options_value = ' true            0.001'
# Most of following are not needed in this input file, but are useful when debugging
#  petsc_options = '-tao_ls_view'
#   petsc_options_iname = '-tao_max_it -tao_max_funcs -tao_ls_max_funcs -tao_ls_ftol -tao_ls_rtol -tao_fd_gradient -tao_fd_delta -tao_gatol -tao_ls_type'
#   petsc_options_value = '20           31             10                1E-2         1E-1         true             1E-5          1E-9       more-thuente'
  verbose = true
[]

[MultiApps]
  [model]
    type = OptimizeFullSolveMultiApp
    reset_app = true # this is necessary in order that model.i starts from temperature=0 each time it is called
    input_files = model.i
    execute_on = FORWARD
  []
[]

[Transfers]
  [diffusivity_to_model]
    type = MultiAppReporterTransfer
    multi_app = model
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/diffusivity_values'
    to_reporters = 'vector_pp/diffusivity_values'
  []
  [toForward_measument]
    type = MultiAppReporterTransfer
    multi_app = model
    direction = to_multiapp
    from_reporters = 'OptimizationReporter/measurement_xcoord OptimizationReporter/measurement_ycoord OptimizationReporter/measurement_zcoord'
    to_reporters = 'measure_data/measurement_xcoord measure_data/measurement_ycoord measure_data/measurement_zcoord'
  []

  [from_model]
    type = MultiAppReporterTransfer
    multi_app = model
    direction = from_multiapp
    from_reporters = 'data_pt/temperature data_pt/temperature vector_pp/diffusivity_values'
    to_reporters = 'OptimizationReporter/simulation_values temperature_at_observation_points/values diffusivities/values'
  []
[]

[Reporters]
  [diffusivities]
    type = ConstantReporter
    real_vector_names = values
    real_vector_values = 0 # dummy initial value
  []
  [temperature_at_observation_points]
    type = ConstantReporter
    real_vector_names = values
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
