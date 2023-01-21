# Test that coupling a time derivative of a scalar variable (ScalarDotCouplingAux) and
# using a Steady executioner errors out

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Functions]
  [./a_fn]
    type = ParsedFunction
    expression = t
  [../]
[]

[AuxVariables]
  [./v]
  [../]

  [./a]
    family = SCALAR
    order = FIRST
  [../]
[]

[AuxScalarKernels]
  [./a_sak]
    type = FunctionScalarAux
    variable = a
    function = a_fn
  [../]
[]

[AuxKernels]
  [./ak_v]
    type = ScalarDotCouplingAux
    variable = v
    v = a
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./td]
    type = TimeDerivative
    variable = u
  [../]
[]

[Executioner]
  type = Steady
[]
