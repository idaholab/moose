[OptimizationReporter]
  type = ParameterMeshOptimization
  objective_name = objective_value
  parameter_names = 'source'
  parameter_meshes = 'initial_param_mesh_in.e'
  initial_condition = '50' # Start with uniform value
  regularization_types = 'L2_GRADIENT'
  regularization_coeffs = '0.001'
  lower_bounds = 1
[]
