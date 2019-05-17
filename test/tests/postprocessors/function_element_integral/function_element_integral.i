dx = 2
y1 = 3
y2 = 6
y3 = 8
integral = ${fparse dx * ((y1 + y2) * 0.5 + (y2 + y3) * 0.5)}

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
  xmax = 4
[]

[Functions]
  [./function]
    type = PiecewiseLinear
    axis = x
    x = '0 2 4'
    y = '${y1} ${y2} ${y3}'
  [../]
[]

[Postprocessors]
  [./integral_pp]
    type = FunctionElementIntegral
    function = function
    execute_on = 'initial'
  [../]
  [./integral_rel_err]
    type = RelativeDifferencePostprocessor
    value1 = integral_pp
    value2 = ${integral}
    execute_on = 'initial'
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
  show = 'integral_rel_err'
[]
