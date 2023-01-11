[Problem]
  type = FEProblem
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 40
  ny = 40
[]

[AuxVariables]
  [u]
    type = MooseVariableFVReal
  []
  [w]
    type = MooseVariableFVReal
  []
  [v_x]
    type = MooseVariableFVReal
  []
  [v_y]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [u]
    type = FunctionAux
    variable = u
    function = u_f
  []
  [w]
    type = FunctionAux
    variable = w
    function = w_f
  []
  [v_x]
    type = FunctionAux
    variable = v_x
    function = v_x_f
  []
  [v_y]
    type = FunctionAux
    variable = v_y
    function = v_y_f
  []
[]

[Functions]
  [u_f] # reaches a maximum value at (0.5, 0.6)
    type = ParsedFunction
    expression = 'sin(pi*x)*sin(pi*y/1.2)'
  []
  [w_f] # reaches a minium expression at (0.7, 0.8)
    type = ParsedFunction
    expression = '-sin(pi*x/1.4)*sin(pi*y/1.6)'
  []

  [v_x_f]
    type = ParsedFunction
    expression = 'x'
  []
  [v_y_f]
    type = ParsedFunction
    expression = 'y'
  []
[]

[Postprocessors]
  [max_u]
    type = ADElementExtremeFunctorValue
    functor = 'u'
  []
  [min_w_f]
    type = ElementExtremeFunctorValue
    functor = 'w_f'
    value_type = min
  []
  [max_v_x]
    type = ADElementExtremeFunctorValue
    functor = 'v_x'
  []
  [min_v_y]
    type = ADElementExtremeFunctorValue
    functor = 'v_y'
    value_type = min
  []

  # because we set v_x and v_y equal to the x and y coordinates, these two postprocessors
  # should just return the point at which u reaches a maximum value
  [max_v_from_proxy_x]
    type = ADElementExtremeFunctorValue
    functor = v_x
    proxy_functor = u
    value_type = max
  []
  [max_v_from_proxy_y]
    type = ADElementExtremeFunctorValue
    functor = v_y
    proxy_functor = u
    value_type = max
  []

  # because we set v_x and v_y equal to the x and y coordinates, these two postprocessors
  # should just return the point at which w reaches a minimum value
  [min_v_from_proxy_x]
    type = ADElementExtremeFunctorValue
    functor = v_x
    proxy_functor = w
    value_type = min
  []
  [min_v_from_proxy_y]
    type = ADElementExtremeFunctorValue
    functor = v_y
    proxy_functor = w
    value_type = min
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
