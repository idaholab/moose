[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 10
  nx = 10
[]

[Functions]
  # The integral of this function is 2*3 + 3*6 + 5*2 = 34
  [test_fn]
    type = PiecewiseConstant
    axis = x
    x = '0 2 5'
    y = '3 6 2'
  []
[]

[Postprocessors]
  [integral_pp]
    type = FunctionElementIntegral
    function = test_fn
    execute_on = 'INITIAL'
  []
  [pp2]
    type = FunctionValuePostprocessor
    function = 6
    execute_on = 'INITIAL'
  []
[]

[AuxVariables]
  [test_var]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[ICs]
  [test_var_ic]
    type = PostprocessorIC
    variable = test_var
    pp1 = integral_pp
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  # This PP should have the sum of the other two PPs: 34 + 6 = 40
  [test_var_pp]
    type = ElementAverageValue
    variable = test_var
    execute_on = 'INITIAL'
  []
[]

[Outputs]
  csv = true
[]
