[OptimizationReporter]
  type = ParameterMeshOptimization
  objective_name = objective_value
  parameter_names = 'source'
  parameter_meshes = initial_param_mesh_in.e
  lower_bounds = 1
  initial_condition_mesh_variable = initialConditions
  regularization_types = 'L2_GRADIENT'
  regularization_coeffs = '0.001'
[]
