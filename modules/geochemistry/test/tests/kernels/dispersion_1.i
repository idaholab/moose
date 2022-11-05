# Dispersion of a step-function front of concentration
# The initial condition is such that the theoretical result is exactly
# c = erf(x/sqrt(4*t*D)), where D = hydrodynamic_dispersion
#
# The finite mesh resolution and large time-step size means this is only achieved approximately (increasing nx and decreasing results in the error decreasing, but note the series approximation to the error function means that the error should never be exactly zero)

por = 2.0 # this is the porosity.  The result should not depend on por in this example since it appears in both terms of the PDE
[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 100
  xmin = -5
  xmax = 5
[]

[Variables]
  [conc]
  []
[]

[ICs]
  [spike]
    type = FunctionIC
    variable = conc
    function = 'if(x<=0.0, -1.0, 1.0)'
  []
[]

[Kernels]
  [dot]
    type = GeochemistryTimeDerivative
    porosity = ${por}
    variable = conc
  []
  [disp]
    type = GeochemistryDispersion
    variable = conc
    porosity = ${por}
    tensor_coeff = '0.3 0 0  0 0 0  0 0 0'
  []
[]

[Executioner]
  type = Transient
  solve_type = Newton
  dt = 0.5
  end_time = 1.0
[]

[AuxVariables]
  [expected]
  []
  [should_be_zero]
  []
[]

[AuxKernels]
  [expected]
    type = FunctionAux
    variable = expected
    function = erf
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
    type = ElementL2Norm
    variable = should_be_zero
  []
[]

[Functions]
  [erf]
    type = ParsedFunction
    # series expansion for evaluating the error function
    expression = 'xi := x / sqrt(4 * t * 0.3); expxi := exp(-xi * xi); if(x < 0.0, -1.0, if(x > 0.0, 1.0, 0.0)) * 2 / sqrt(pi) * sqrt(1 - expxi) * (sqrt(pi) / 2.0 + 31.0 * expxi / 200.0 - 341.0 * expxi * expxi / 8000.0)'
  []
[]

[Outputs]
  exodus = true
  execute_on = final
[]
