!include forcing_function_test.i

[Functions]
  active = 'wrong_forcing_function'
  [wrong_forcing_function]
    type = ParsedFunction
    expression = '"alpha*alpha*pi*pi*sin(alpha*pi*x)"'
    symbol_names = 'alpha'
    symbol_values = '16'
  []
[]
