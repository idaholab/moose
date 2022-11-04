[ICs]
  active = 'mms'
  [mms]
    type = FunctionIC
    variable = T
    function = mms_exact
  []
[]

[BCs]
  active = 'mms'
  [mms]
    type = FunctionDirichletBC
    variable = T
    boundary = 'left right top bottom'
    function = mms_exact
  []
[]

[Kernels]
  [mms]
    type = HeatSource
    variable = T
    function = mms_force
  []
[]

[Functions]
  [mms_force]
    type = ParsedFunction
    expression = '-3.08641975308642e-5*x*y*cp*rho*exp(-3.08641975308642e-5*t) - shortwave*exp(y*kappa)*sin((1/2)*x*pi)*sin((1/3600)*pi*t/hours)'
    symbol_names = 'rho cp   k     kappa shortwave hours'
    symbol_values = '150 2000 0.01  40    650       9'
  []
  [mms_exact]
    type = ParsedFunction
    expression = 'x*y*exp(-3.08641975308642e-5*t)'
  []
[]

[Outputs]
  csv = true
[]

[Postprocessors]
  [error]
    type = ElementL2Error
    variable = T
    function = mms_exact
  []
  [delta_t]
    type = TimestepSize
  []
[]
