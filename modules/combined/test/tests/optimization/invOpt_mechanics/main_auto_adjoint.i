[Optimization]
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 2
  xmin = 0.0
  xmax = 5.0
  ymin = 0.0
  ymax = 1.0
[]

[OptimizationReporter]
  type = GeneralOptimization
  objective_name = objective_value
  parameter_names = 'force_right'
  num_values = '2'
  initial_condition = '100'
  outputs = 'csv'
[]

[Executioner]
  type = Optimize
  tao_solver = taobqnktr
  petsc_options_iname = '-tao_gttol -tao_max_it'
  petsc_options_value = '1e-7 500 '
  verbose = true
[]

[MultiApps]
  [forward]
    type = FullSolveMultiApp
    input_files = forward_and_adjoint.i
    execute_on = "FORWARD"
    clone_parent_mesh = true
  []
[]

[Transfers]
  [toForward_measument]
    type = MultiAppReporterTransfer
    to_multi_app = forward
    from_reporters = 'OptimizationReporter/force_right'
    to_reporters = 'params/right_values'
  []
  [fromForward]
    type = MultiAppReporterTransfer
    from_multi_app = forward
    from_reporters = 'combined/gradient obj/obj_val'
    to_reporters = 'OptimizationReporter/grad_force_right OptimizationReporter/objective_value'
  []
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
