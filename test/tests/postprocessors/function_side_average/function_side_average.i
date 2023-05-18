A = 2
B = 5

x2 = 4
y2 = 3

integral_exact = ${fparse A * x2 * y2 + 0.5 * B * y2^2}
avg_exact = ${fparse integral_exact / y2}

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmax = ${x2}
  ymax = ${y2}
[]

[Functions]
  [test_fn]
    type = ParsedFunction
    expression = '${A}*x + ${B}*y'
  []
[]

[Postprocessors]
  [avg]
    type = FunctionSideAverage
    boundary = 'right'
    function = test_fn
    execute_on = 'INITIAL'
  []
  [avg_err]
    type = RelativeDifferencePostprocessor
    value1 = avg
    value2 = ${avg_exact}
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
  show = 'avg_err'
[]
