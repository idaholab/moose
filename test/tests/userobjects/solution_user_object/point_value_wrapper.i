[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 1
  ny = 1
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [dummy]
    type = Reaction
    variable = u
  []
[]

[UserObjects]
  [soln]
    type = SolutionUserObject
    mesh = discontinuous_value_solution_uo_p1.e
    system_variables = 'discontinuous_variable continuous_variable'
    timestep = LATEST
  []
[]

[Postprocessors]
  [scalar_average_face]
    type = TestSolutionPointValueWrapper
    variable = discontinuous_variable
    point = '0.5 0.25 0'
    weighting_type = average
    solution = soln
    execute_on = INITIAL
  []

  [scalar_smallest_face]
    type = TestSolutionPointValueWrapper
    variable = discontinuous_variable
    point = '0.5 0.25 0'
    weighting_type = smallest_element_id
    solution = soln
    execute_on = INITIAL
  []

  [scalar_largest_face]
    type = TestSolutionPointValueWrapper
    variable = discontinuous_variable
    point = '0.5 0.25 0'
    weighting_type = largest_element_id
    solution = soln
    execute_on = INITIAL
  []

  [scalar_largest_elem_zero]
    type = TestSolutionPointValueWrapper
    variable = discontinuous_variable
    point = '0.25 0.25 0'
    weighting_type = largest_element_id
    solution = soln
    execute_on = INITIAL
  []

  [gradient_average_face]
    type = TestSolutionPointValueWrapper
    variable = continuous_variable
    point = '0.5 0.25 0'
    evaluate_gradient = true
    gradient_component = x
    weighting_type = average
    solution = soln
    execute_on = INITIAL
  []

  [gradient_smallest_face]
    type = TestSolutionPointValueWrapper
    variable = continuous_variable
    point = '0.5 0.25 0'
    evaluate_gradient = true
    gradient_component = x
    weighting_type = smallest_element_id
    solution = soln
    execute_on = INITIAL
  []

  [gradient_largest_face]
    type = TestSolutionPointValueWrapper
    variable = continuous_variable
    point = '0.5 0.25 0'
    evaluate_gradient = true
    gradient_component = x
    weighting_type = largest_element_id
    solution = soln
    execute_on = INITIAL
  []

  [gradient_largest_elem_zero]
    type = TestSolutionPointValueWrapper
    variable = continuous_variable
    point = '0.25 0.25 0'
    evaluate_gradient = true
    gradient_component = x
    weighting_type = largest_element_id
    solution = soln
    execute_on = INITIAL
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = INITIAL
[]
