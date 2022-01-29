[Mesh]
  type = GeneratedMesh
  dim = 1
  xmin = 0
  xmax = 10
  nx = 10
[]

[Functions]
  # Integral of this function should be 2*3 + 3*6 + 5*2 = 34
  [test_fn]
    type = PiecewiseConstant
    axis = x
    x = '0 2 5'
    y = '3 6 2'
  []
[]

[UserObjects]
  [test_uo]
    type = FunctionElementLoopIntegralUserObject
    function = test_fn
    execute_on = 'INITIAL'
  []
[]

[Postprocessors]
  [test_pp]
    type = FunctionElementLoopIntegralGetValueTestPostprocessor
    function_element_loop_integral_uo = test_uo
    execute_on = 'INITIAL'
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
