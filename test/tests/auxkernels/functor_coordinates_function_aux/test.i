[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 3
  ny = 3
  elem_type = QUAD9
[]

[AuxVariables]
  # These aux-variables are used as argument functors to the FunctorAbuseAux
  [x_var]
    initial_condition = 1
  []
  [y_var]
    type = MooseVariableFVReal
  []
  [temperature]
    initial_condition = 300
  []

  # These are example output variables computed by the FunctorAbuseAux
  [density]
  []
  [multiple_dependency_test]
    type = MooseVariableFVReal
  []
  [higher_order_test]
    family = MONOMIAL
    order = SECOND
  []
[]

[Functions]
  [density_correlation]
    type = ParsedFunction
    expression = '900 - t'
  []
  [multi_dependency]
    type = ParsedFunction
    expression = 't*((x*x)+(y*y) + z)'
  []
  [y_function]
    type = ParsedFunction
    expression = 'y'
  []

  # Function used as a functor for the test
  [time_function]
    type = ParsedFunction
    expression = '1 + 2 * t'
  []
[]

[Postprocessors]
  # Postprocessor used as a functor for the test
  [z_pp]
    type = Receiver
    default = 30
  []
[]

[AuxKernels]
  [set_density]
    type = FunctorCoordinatesFunctionAux
    variable = density
    function = density_correlation
    t_functor = 'temperature'
    x_functor = 0
    y_functor = 0
    z_functor = 0
  []
  [set_y]
    type = FunctorAux
    functor = 'y_function'
    variable = 'y_var'
    # this auxkernel must execute before the y_var functor is used
    # in the FunctorCoordinatesFunctionAux if we want y to be up to date!
    execute_on = 'INITIAL'
  []
  [set_complex_dependency_fv]
    type = FunctorCoordinatesFunctionAux
    variable = multiple_dependency_test
    function = multi_dependency
    t_functor = 'time_function'
    x_functor = 'x_var'
    y_functor = 'y_var'
    z_functor = 'z_pp'
  []
  [set_complex_dependency_higher_order]
    type = FunctorCoordinatesFunctionAux
    variable = higher_order_test
    function = multi_dependency
    t_functor = 'time_function'
    x_functor = 'x_var'
    y_functor = 'y_var'
    z_functor = 'z_pp'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
  hide = 'x_var y_var z_pp temperature'
[]
