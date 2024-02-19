[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 3.1416
    ymin = 0
    ymax = 3.1416
    nx = 10
    ny = 10
  []
[]

[Variables]
  [u]
    type = MooseVariableFVReal
  []
[]

[Functions]
  [parsed_function]
    type = ParsedFunction
    expression = 'sin(x)-cos(y/2)'
  []
[]

[FVICs]
  [u_ic]
    type = FVFunctionIC
    variable = 'u'
    function = parsed_function
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
[]
