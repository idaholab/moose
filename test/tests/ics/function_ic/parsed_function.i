#
# Test the automatically generated gradients in ParsedFunction and the gradient pass-through in FunctionIC
# OLD MOOSE behavior was for parsed_function to behave the same as parsed_zerograd_function
# NEW MOOSE behavior is for parsed_function to behave the same as parsed_grad_function
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 3.141
  ymin = 0
  ymax = 3.141
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
    order = THIRD
    family = HERMITE
  [../]
[]

[Functions]
  [./parsed_function]
    type = ParsedFunction
    expression = 'sin(x)-cos(y/2)'
  [../]
  [./parsed_grad_function]
    type =ParsedGradFunction
    expression = 'sin(x)-cos(y/2)'
    grad_x = 'cos(x)'
    grad_y = 'sin(y/2)/2'
  [../]
  [./parsed_zerograd_function]
    type = ParsedGradFunction
    value = 'sin(x)-cos(y/2)'
    grad_x = '0'
    grad_y = '0'
  [../]
[]

[ICs]
  [./u_ic]
    type = FunctionIC
    variable = 'u'
    function = parsed_function
  [../]
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  file_base = parsed
  [./OverSampling]
    type = Exodus
    refinements = 3
  [../]
[]
