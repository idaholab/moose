[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmin = 0
  xmax = 3.141
  ymin = 0
  ymax = 3.141
[]

[Variables]
  [u]
  []
  [v]
  []
  [w]
    type = MooseVariableFVReal
  []
[]

[ICs]
  [u_ic]
    type = FunctionIC
    variable = 'u'
    function = parsed_function
  []
  [v_ic]
    type = FunctionIC
    variable = 'v'
    function = 'x'
  []
  [w_ic]
    type = FunctionIC
    variable = 'w'
    function = 'x + y'
  []
[]

[Functions]
  [parsed_function]
    type = ParsedFunction
    value = 'sin(x)-cos(y/2)'
  []
  [parsed_grad_function]
    type = ParsedVectorFunction
    expression_x = 'cos(x)'
    expression_y = 'sin(y/2)/2'
  []
  [parsed_gradx_function]
    type = ParsedFunction
    value = 'cos(x)'
  []
[]

[AuxVariables]
  [funcGrad_u]
    order = CONSTANT
    family = MONOMIAL_VEC
  []
  [auxGrad_u]
    order = CONSTANT
    family = MONOMIAL_VEC
  []
  [auxGrad_v]
    order = CONSTANT
    family = MONOMIAL_VEC
  []
  [auxGrad_fv]
    order = CONSTANT
    family = MONOMIAL_VEC
  []
  [auxGrad_function]
    order = CONSTANT
    family = MONOMIAL_VEC
  []

  [funcGrad_u_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [auxGrad_u_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [auxGrad_v_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [auxGrad_fv_x]
    order = CONSTANT
    family = MONOMIAL
  []
  [auxGrad_function_x]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  # Verification
  [vec]
    type = VectorFunctionAux
    variable = funcGrad_u
    function = parsed_grad_function
  []

  # Finite element variables with and without scaling by material
  [grad_u]
    type = ADFunctorElementalGradientAux
    variable = auxGrad_u
    functor = u
  []
  [grad_v]
    type = ADFunctorElementalGradientAux
    variable = auxGrad_v
    functor = v
    factor_matprop = 'trig_material'
  []

  # Finite volume variable
  [grad_w]
    type = ADFunctorElementalGradientAux
    variable = auxGrad_fv
    functor = w
    factor = w
  []

  # Functions
  [grad_function]
    type = FunctorElementalGradientAux
    variable = auxGrad_function
    functor = parsed_gradx_function
  []

  # Output a component, line sampler does not do vector variables
  [funcGrad_u_x]
    type = VectorVariableComponentAux
    variable = funcGrad_u_x
    vector_variable = funcGrad_u
    component = 'x'
  []
  [auxGrad_u_x]
    type = VectorVariableComponentAux
    variable = auxGrad_u_x
    vector_variable = auxGrad_u
    component = 'x'
  []
  [auxGrad_v_x]
    type = VectorVariableComponentAux
    variable = auxGrad_v_x
    vector_variable = auxGrad_v
    component = 'x'
  []
  [funcGrad_fv_x]
    type = VectorVariableComponentAux
    variable = auxGrad_fv_x
    vector_variable = auxGrad_fv
    component = 'x'
  []
  [auxGrad_function_x]
    type = VectorVariableComponentAux
    variable = auxGrad_function_x
    vector_variable = auxGrad_function
    component = 'x'
  []
[]

[Materials]
  [steel]
    type = ADGenericFunctionMaterial
    prop_names = 'trig_material'
    prop_values = 'parsed_gradx_function'
  []
[]

[VectorPostprocessors]
  [results]
    type = LineValueSampler
    start_point = '0 1 0'
    end_point = '3.141 1 0'
    variable = 'funcGrad_u_x auxGrad_u_x auxGrad_v_x auxGrad_fv_x auxGrad_function_x'
    num_points = 20
    sort_by = x
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
[]
