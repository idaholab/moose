[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
  []
[]

[Variables]
  [u][]
[]

[AuxVariables]
  [nodal][]
  [elemental]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [nodal]
    type = FunctorAux
    functor = u
    variable = nodal
  []
  [elemental]
    type = FunctorAux
    functor = u
    variable = elemental
  []
[]

[Functions]
  [analytic]
    type = ParsedFunction
    expression = 'x*t'
  []
[]

[NodalKernels]
  [rxn]
    type = ReactionNodalKernel
    variable = u
  []
  [ffn]
    type = UserForcingFunctionNodalKernel
    variable = u
    function = analytic
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  num_steps = 1
  dt = 1
[]

[Postprocessors]
  [u_err]
    type = ElementL2FunctorError
    approximate = u
    exact = analytic
  []
  [nodal_err]
    type = ElementL2FunctorError
    approximate = nodal
    exact = analytic
  []
  [elemental_err]
    type = ElementL2FunctorError
    approximate = elemental
    exact = analytic
  []
[]

[Outputs]
  csv = true
[]
