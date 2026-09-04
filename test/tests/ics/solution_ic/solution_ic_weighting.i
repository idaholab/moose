[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 2
    ny = 1
  []
[]

[Variables]
  [u_average]
    family = LAGRANGE
    order = FIRST
  []
  [u_smallest]
    family = LAGRANGE
    order = FIRST
  []
  [u_largest]
    family = LAGRANGE
    order = FIRST
  []
[]

[UserObjects]
  [source_solution]
    type = SolutionUserObject
    mesh = solution_ic_weighting_source_out.e
    system_variables = source_value
    timestep = LATEST
  []
[]

[ICs]
  [average]
    type = SolutionIC
    solution_uo = source_solution
    variable = u_average
    from_variable = source_value
    weighting_type = average
  []

  [smallest]
    type = SolutionIC
    solution_uo = source_solution
    variable = u_smallest
    from_variable = source_value
    weighting_type = smallest_element_id
  []

  [largest]
    type = SolutionIC
    solution_uo = source_solution
    variable = u_largest
    from_variable = source_value
    weighting_type = largest_element_id
  []
[]

[Postprocessors]
  [average_value]
    type = PointValue
    variable = u_average
    point = '0.5 0.5 0'
    execute_on = INITIAL
  []

  [smallest_value]
    type = PointValue
    variable = u_smallest
    point = '0.5 0.5 0'
    execute_on = INITIAL
  []

  [largest_value]
    type = PointValue
    variable = u_largest
    point = '0.5 0.5 0'
    execute_on = INITIAL
  []
[]

[Problem]
  kernel_coverage_check = false
  skip_nl_system_check = true
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  execute_on = INITIAL
[]
