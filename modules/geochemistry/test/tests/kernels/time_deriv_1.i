# An initial concentration field in a material with constant porosity is subjected to a constant source
# porosity * d(concentration)/dt = source
# The result is checked vs the expected solution, which is conc = conc_old + dt * source / porosity
[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 4
  nz = 2
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
  [source]
    type = BodyForce
    function = 3.0
    variable = conc
  []
[]

[ICs]
  [conc]
    type = FunctionIC
    function = 'z * z + 4 * x * x * x + y'
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
    function = '6.0'
    variable = porosity
  []
  [expected]
    type = FunctionAux
    function = 'z * z + 4 * x * x * x + y + 2.0 * 3.0 / 6.0'
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

