[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 2
  []
[]

[Functions]
  [linear]
    type = ParsedFunction
    expression = 'x + 10*t + 2*t*t'
  []
[]

[Variables]
  [base]
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = base
  []
  [source]
    type = BodyForce
    variable = base
    function = linear
  []
[]

[AuxVariables]
  [first]
  []
  [first_from_ad]
  []
[]

[AuxKernels]
  [set_first]
    type = DotCouplingAux
    variable = first
    v = base
  []
  [set_first_from_AD]
    type = ADDotCouplingAux
    variable = first_from_ad
    v = base
  []
[]

[Executioner]
  type = Transient
  num_steps = 3
[]

[Postprocessors]
  [dot]
    type = AverageNodalVariableValue
    variable = 'first'
  []
  [ad_dot]
    type = AverageNodalVariableValue
    variable = 'first_from_ad'
  []
  # Ideally we would test the second derivative here, but it's not implemented yet for nodal variables
  # through the coupleable API
[]

[Outputs]
  csv = true
[]
