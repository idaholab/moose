[Mesh]
  file = ../../mesh/square_quad.e
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Variables]
  [libmesh_scalar_var]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Functions]
  [parsed_function]
    type = ParsedFunction
    expression = 'x*x + y*y'
  []
[]

[ICs]
  [libmesh_scalar_var_ic]
    type = FunctionIC
    variable = 'libmesh_scalar_var'
    function = parsed_function
  []
[]

[Executioner]
  type = Steady
[]
