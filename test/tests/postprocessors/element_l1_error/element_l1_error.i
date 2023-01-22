# Tests the ElementL1Error post-processor.
#
# The Element L1 error is defined as follows:
#   \sum\limits_i = int\limits_{\Omega_i} |y_{h,i} - y(x)| d\Omega
# where i is the element index and y_h is the approximate solution.
#
# This example uses 2 uniform elements on (0,10) with the following values:
#   (0,5):  y = 3, y_h = 5
#   (5,10): y = 2, y_h = 6
# Thus the gold value is
#   gold = 5*(5-3) + 5*(6-2) = 10 + 20 = 30

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
  xmin = 0
  xmax = 10
[]

[Variables]
  [u]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[ICs]
  [u_ic]
    type = FunctionIC
    variable = u
    function = u_ic_fn
  []
[]

[Functions]
  [u_ic_fn]
    type = ParsedFunction
    expression = 'if(x<5,5,6)'
  []

  [u_exact_fn]
    type = ParsedFunction
    expression = 'if(x<5,3,2)'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [err]
    type = ElementL1Error
    variable = u
    function = u_exact_fn
    execute_on = 'initial'
  []
[]

[Outputs]
  csv = true
  execute_on = 'initial'
[]
