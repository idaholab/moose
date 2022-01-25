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
[]

[ICs]
  [./u_ic]
    type = FunctionIC
    variable = 'u'
    function = parsed_function
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
[]

[AuxKernels]
  [vec]
  type = VectorFunctionAux
  variable = funcGrad_u
  function = parsed_grad_function
  [../]
  [gradTx]
    type = ScaledGradientVector
    gradient_variable = u
    variable = auxGrad_u
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
  exodus = true
[]
