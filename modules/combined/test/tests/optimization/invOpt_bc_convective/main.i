[Optimization]
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 20
    xmax = 1
    ymax = 2
  []
[]

[OptimizationReporter]
  type = OptimizationReporter
  parameter_names = 'p1'
  num_values = '1'
  initial_condition = '9'
  upper_bounds = '10'
  lower_bounds = '1'
  measurement_points = '0.1  0  0
                        0.1  0.1  0
                        0.1  0.2  0
                        0.1  0.3  0
                        0.1  0.4  0
                        0.1  0.5  0
                        0.1  0.6  0
                        0.1  0.7  0
                        0.1  0.8  0
                        0.1  0.9  0
                        0.1  1  0
                        0.1  1.1  0
                        0.1  1.2  0
                        0.1  1.3  0
                        0.1  1.4  0
                        0.1  1.5  0
                        0.1  1.6  0
                        0.1  1.7  0
                        0.1  1.8  0
                        0.1  1.9  0
                        0.1  2    0'
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
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward.i
    execute_on = "FORWARD"
    clone_parent_mesh = true
  []
  [adjoint]
    type = FullSolveMultiApp
    input_files = adjoint.i
    execute_on = "ADJOINT"
    clone_parent_mesh = true
  []
[]

[Transfers]
  #these are usually the same for all input files.
  [toForward]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/measurement_values
                      OptimizationReporter/p1'
    to_reporters = 'measure_data/measurement_xcoord
                    measure_data/measurement_ycoord
                    measure_data/measurement_zcoord
                    measure_data/measurement_time
                    measure_data/measurement_values
                    params/vals'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'measure_data/simulation_values'
    to_reporters = 'OptimizationReporter/simulation_values'
  []
  [toAdjoint]
    type = MultiAppReporterTransfer
    to_multi_app = adjoint
    from_reporters = 'OptimizationReporter/measurement_xcoord
                      OptimizationReporter/measurement_ycoord
                      OptimizationReporter/measurement_zcoord
                      OptimizationReporter/measurement_time
                      OptimizationReporter/misfit_values
                      OptimizationReporter/p1'
    to_reporters = 'misfit/measurement_xcoord
                    misfit/measurement_ycoord
                    misfit/measurement_zcoord
                    misfit/measurement_time
                    misfit/misfit_values
                    params/vals'
  []
  [fromAdjoint]
    type = MultiAppReporterTransfer
    from_multi_app = adjoint
    from_reporters = 'adjoint_pt/inner_product'
    to_reporters = 'OptimizationReporter/grad_p1'
  []

  # these are transferring data from subapp to subapp because the adjoint problem
  # needs the forward solution to compute the gradient.  Maybe this step could be
  # done on the main app.  The adjoint only passes the adjoint variable (whole mesh)
  # to the main app and the main app computes the gradient from this.
  [fromForwardtoAdjoint_temp]
    type = MultiAppCopyTransfer
    from_multi_app = forward
    to_multi_app = adjoint
    source_variable = 'temperature'
    variable = 'temperature_forward'
  []
[]

[Outputs]
  csv = true
[]
