[OptimizationReporter]
  type = ParameterMeshOptimization
  objective_name = objective_value
  parameter_names = 'source'
  parameter_meshes = 'create_parameter_mesh_out.e'
  initial_condition = '50' # Start with uniform value
  regularization_types = 'L2_GRADIENT'
  regularization_coeffs = '0.001'
  lower_bounds = 1
[]
