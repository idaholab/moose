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
    system_variables = 'source_piecewise_constant source_piecewise_linear'
    timestep = LATEST
  []
[]

[Functions]
  [weighted_piecewise_constant]
    type = SolutionFunction
    solution = source_solution
    from_variable = source_piecewise_constant
    weighting_type = average
  []

  [weighted_piecewise_linear]
    type = SolutionFunction
    solution = source_solution
    from_variable = source_piecewise_linear
    weighting_type = average
  []
[]

[AuxKernels]
  [evaluate_gradient_x]
    type = FunctionDerivativeAux
    variable = gradient_x
    function = weighted_piecewise_linear
    component = x
    execute_on = INITIAL
  []
[]

[Postprocessors]
  [function_value]
    type = FunctionValuePostprocessor
    function = weighted_piecewise_constant
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
