# This performs a 3D material inversion problem
# in a bi-material domain
#
[Optimization]
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = -2
    xmax = 2
    ymin = -2
    ymax = 2
    zmin = 0
    zmax = 2
    nx = 16
    ny = 16
    nz = 8
  []
[]

[OptimizationReporter]
  type = ParameterMeshOptimization
  parameter_names = 'diffusivity_values'
  parameter_meshes = parameter_mesh_in.e
  num_parameter_times = 1
  parameter_families = MONOMIAL
  parameter_orders = CONSTANT
  initial_condition = '1'
  lower_bounds = '1'
  upper_bounds = '20'
  measurement_file = 'synthetic_data.csv'
  file_value = 'temperature'
  file_xcoord = x
  file_ycoord = y
  file_zcoord = z
[]

[Executioner]
  type = Optimize
  tao_solver = taoblmvm
  petsc_options_iname = '-tao_gatol'
  petsc_options_value = '1e-3'
  verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = model.i
    execute_on = FORWARD
  []
  [adjoint]
    type = FullSolveMultiApp
    input_files = grad.i #write this input file to compute the adjoint solution and the gradient
    execute_on = ADJOINT
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
    from_reporters = 'measure_data/simulation_values'
    to_reporters = 'OptimizationReporter/simulation_values'
  []
  #############
  #copy the temperature variable - we will need this for the computation of the gradient
  [fromforwardMesh]
    type = MultiAppCopyTransfer
    from_multi_app = forward
    to_multi_app = adjoint
    source_variable = 'temperature'
    variable = 'temperature_forward'
  []
  #############
  [toAdjoint] #pass the misfit to the adjoint
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/misfit_values
                      OptimizationReporter/diffusivity_values'
    to_reporters = 'misfit/measurement_xcoord
                    misfit/measurement_ycoord
                    misfit/measurement_zcoord
                    misfit/measurement_time
                    misfit/misfit_values
                    data/diffusivity'
  []
  [fromadjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'gradvec/inner_product'
    to_reporters = 'OptimizationReporter/grad_diffusivity_values'
  []
[]

[Outputs]
  csv = true
[]
