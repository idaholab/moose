var1 = 1
var2 = '${fparse var1 / 10}'

[StochasticTools]
[]

[Postprocessors]
  [var1]
    type = ConstantPostprocessor
    value = ${var1}
    execute_on = initial
  []
  [var2]
    type = ConstantPostprocessor
    value = ${var2}
    execute_on = initial
  []
[]

[UserObjects]
  [check]
    type = Terminator
    expression = 'var1 < 1 | var2 > 1'
    error_level = ERROR
  []
[]
