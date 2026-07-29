[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
  []
[]

[Problem]
  kernel_coverage_check = false
  skip_nl_system_check = true
  solve = false
[]

[UserObjects]
  [fixed_solution]
    type = SolutionUserObject
    mesh = point_value_wrapper_time_source_out.e
    system_variables = 'source_value source_gradient'
    timestep = LATEST
  []

  [interpolated_solution]
    type = SolutionUserObject
    mesh = point_value_wrapper_time_source_out.e
    system_variables = 'source_value source_gradient'
  []
[]

[Postprocessors]
  [scalar_restricted_latest]
    type = TestSolutionPointValueWrapper
    variable = source_value
    point = '0.5 0.5 0'
    weighting_type = average
    source_subdomain_ids = '1'
    solution = fixed_solution
    execute_on = INITIAL
  []

  [scalar_restricted_interpolated]
    type = TestSolutionPointValueWrapper
    variable = source_value
    point = '0.5 0.5 0'
    weighting_type = average
    source_subdomain_ids = '1'
    solution = interpolated_solution
    execute_on = INITIAL
  []

  [gradient_average_interpolated]
    type = TestSolutionPointValueWrapper
    variable = source_gradient
    point = '0.5 0.5 0'
    evaluate_gradient = true
    gradient_component = x
    weighting_type = average
    solution = interpolated_solution
    execute_on = INITIAL
  []
[]

[Executioner]
  type = Transient
  start_time = 0.5
  num_steps = 0
[]

[Outputs]
  csv = true
  execute_on = INITIAL
[]
