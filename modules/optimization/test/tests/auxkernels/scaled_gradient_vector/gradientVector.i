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
[]

[ICs]
  [./u_ic]
    type = FunctionIC
    variable = 'u'
    function = parsed_function
  [../]
  [./v_ic]
    type = FunctionIC
    variable = 'v'
    function = 'x'
  [../]
[]

[Functions]
  [./parsed_function]
    type = ParsedFunction
    value = 'sin(x)-cos(y/2)'
  [../]
  [./parsed_grad_function]
    type = ParsedVectorFunction
    value_x = 'cos(x)'
    value_y = 'sin(y/2)/2'
  [../]
  [./parsed_gradx_function]
    type = ParsedFunction
    value = 'cos(x)'
  [../]
[]

[AuxVariables]
  [./funcGrad_u]
    order = CONSTANT
    family = MONOMIAL_VEC
  [../]
  [./auxGrad_u]
    order = CONSTANT
    family = MONOMIAL_VEC
  [../]
  [./auxGrad_v]
    order = CONSTANT
    family = MONOMIAL_VEC
  [../]
  [./funcGrad_u_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./auxGrad_u_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./auxGrad_v_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
[]

[AuxKernels]
  [vec]
    type = VectorFunctionAux
    variable = funcGrad_u
    function = parsed_grad_function
  [../]
  [grad_u]
    type = ScaledGradientVector
    gradient_variable = u
    variable = auxGrad_u
  []
  [grad_v]
    type = ScaledGradientVector
    gradient_variable = v
    variable = auxGrad_v
    material_scaling = 'trig_material'
  []
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
[]

[Materials]
  [steel]
    type = GenericFunctionMaterial
    prop_names = 'trig_material'
    prop_values = 'parsed_gradx_function'
  []
[]

[VectorPostprocessors]
 [results]
   type =  LineValueSampler
   start_point = '0 1 0'
   end_point = '3.141 1 0'
   variable = 'funcGrad_u_x auxGrad_u_x auxGrad_v_x'
   num_points = 20
   sort_by =  x
 []
[]

[Problem]
  solve = false
[]
[Executioner]
  type = Steady
[]

[Outputs]
  # console = true
  csv = true
[]
