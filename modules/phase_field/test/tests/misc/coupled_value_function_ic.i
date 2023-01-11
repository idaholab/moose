[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 10
    ny = 10
  []
[]

# Here we sum up the inverses of the ICs above. This should add up to 2.0 everywhere
[Functions]
  [map]
    type = ParsedFunction
    expression = 'x^2+y^3+log(z)+acos(t)'
  []
[]

[Variables]
  [out]
    [InitialCondition]
      type = CoupledValueFunctionIC
      function = map
      v = 'v1 v2 a3 a4'
    []
  []
  [v1]
    [InitialCondition]
      type = FunctionIC
      function = x^(1/2)
    []
  []
  [v2]
    [InitialCondition]
      type = FunctionIC
      function = y^(1/3)
    []
  []
[]

[AuxVariables]
  [a3]
    [InitialCondition]
      type = FunctionIC
      function = exp(1-x)
    []
  []
  [a4]
    [InitialCondition]
      type = FunctionIC
      function = cos(1-y)
    []
  []
[]

[Postprocessors]
  [out_min]
    type = ElementExtremeValue
    variable = out
    value_type = min
  []
  [out_max]
    type = ElementExtremeValue
    variable = out
    value_type = max
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  execute_on = 'FINAL'
  csv = true
[]
