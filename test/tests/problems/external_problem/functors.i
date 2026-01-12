[Mesh]
  [m]
    type = CartesianMeshGenerator
    dim = 1
    dx = '10.0'
    ix = 2
  []
[]

[AuxVariables]
  [test]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [test]
    type = ParsedAux
    variable = test
    expression = 'x'
    use_xyzt = true
  []
[]

[Postprocessors]
  [no_functor]
    type = ElementIntegralVariablePostprocessor
    variable = test
  []
  [functor]
    type = ADElementIntegralFunctorPostprocessor
    functor = test
  []
[]

[Problem]
  type = DummyExternalProblem
[]

[Executioner]
  type = Transient
  num_steps = 2
[]

[Outputs]
  csv = true
[]
