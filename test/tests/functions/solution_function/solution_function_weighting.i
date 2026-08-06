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

[AuxVariables]
  [gradient_x]
    family = LAGRANGE
    order = FIRST
  []
[]

[UserObjects]
  [source_solution]
    type = SolutionUserObject
    mesh = solution_function_weighting_source_out.e
    system_variables = 'source_value source_gradient'
    timestep = LATEST
  []
[]

[Functions]
  [weighted_value]
    type = SolutionFunction
    solution = source_solution
    from_variable = source_value
    weighting_type = average
  []

  [weighted_gradient]
    type = SolutionFunction
    solution = source_solution
    from_variable = source_gradient
    weighting_type = average
  []
[]

[AuxKernels]
  [evaluate_gradient_x]
    type = FunctionDerivativeAux
    variable = gradient_x
    function = weighted_gradient
    component = x
    execute_on = INITIAL
  []
[]

[Postprocessors]
  [function_value]
    type = FunctionValuePostprocessor
    function = weighted_value
    point = '0.5 0.5 0'
    execute_on = INITIAL
  []

  [function_gradient_x]
    type = PointValue
    variable = gradient_x
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
