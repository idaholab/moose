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
    expression = 'cp*rho*sin(x*pi)*sin(5*y*pi) + 26*pi^2*k*t*sin(x*pi)*sin(5*y*pi) - shortwave*exp(y*kappa)*sin((1/2)*x*pi)*sin((1/3600)*pi*t/hours)'
    symbol_names = 'rho cp   k     kappa shortwave hours'
    symbol_values = '150 2000 0.01  40    650       9'
  []
  [mms_exact]
    type = ParsedFunction
    expression = 't*sin(pi*x)*sin(5*pi*y)'
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
  [h]
    type = AverageElementSize
  []
[]
