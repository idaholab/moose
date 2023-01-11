# A point-source is added to fluid in a material with spatially-varying porosity
# porosity * d(concentration)/dt = 3.0 * delta(x - 1.0)
# where delta is the Dirac delta function (a ConstantPointSource DiracKernel)
# The solution, at x = 1.0 is
# concentration = concentration_old + 3 * dt / porosity
# while concentration is unchanged elsewhere.
# Note that since GeochemistryTimeDerivative is mass-lumped, it produces this solution.
# If mass lumping had not been used, concentration would have decreased at x != 1.0
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
  xmax = 2
[]

[Variables]
  [conc]
  []
[]

[Kernels]
  [dot]
    type = GeochemistryTimeDerivative
    porosity = porosity
    variable = conc
  []
[]

[DiracKernels]
  [source]
    type = ConstantPointSource
    point = '1.0 0 0'
    variable = conc
    value = 12.0
  []
[]

[ICs]
  [conc]
    type = FunctionIC
    function = 'x * x'
    variable = conc
  []
[]

[AuxVariables]
  [porosity]
  []
  [expected]
  []
  [should_be_zero]
  []
[]

[AuxKernels]
  [porosity]
    type = FunctionAux
    function = '6.0 + x'
    variable = porosity
  []
  [expected]
    type = FunctionAux
   function = 'if(x > 0.5 & x < 1.5, x * x + 2.0 * 12.0 / (6.0 + x), x * x)'
    variable = expected
  []
  [should_be_zero]
    type = ParsedAux
    coupled_variables = 'expected conc'
    expression = 'expected - conc'
    variable = should_be_zero
  []
[]

[Postprocessors]
  [error]
    type = NodalL2Norm
    variable = should_be_zero
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 2
  end_time = 2
[]

[Outputs]
  csv = true
[]

