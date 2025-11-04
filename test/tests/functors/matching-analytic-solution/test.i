[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 20
  []
[]

[Variables]
  [u]
  []
[]

[AuxVariables]
  [nodal]
  []
  [elemental]
    type = MooseVariableFVReal
  []
  [elemental_grad]
    type = MooseVariableFVReal
  []
  [elemental_dot]
    type = MooseVariableFVReal
  []
  [elemental_grad_dot]
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
  [elemental_dot]
    type = FunctorAux
    functor = dmat_dt
    variable = elemental_dot
  []
  [elemental_grad]
    type = FunctorVectorElementalAux
    functor = grad_mat
    component = 0
    variable = elemental_grad
  []
  [elemental_grad_dot]
    type = FunctorVectorElementalAux
    functor = grad_dmat_dt
    component = 0
    variable = elemental_grad_dot
  []
[]

[Functions]
  [analytic]
    type = ParsedFunction
    expression = 'x*t'
  []
  [grad]
    type = ParsedFunction
    expression = 't'
  []
  [dot]
    type = ParsedFunction
    expression = 'x'
  []
  [grad_dot]
    type = ParsedFunction
    expression = '1'
  []
[]

[FunctorMaterials]
  [val_dot_grad_dot]
    type = ADGenericFunctorMaterial
    prop_names = 'mat'
    prop_values = 'u'
  []
  [grad]
    type = ADGenericFunctorGradientMaterial
    prop_names = 'grad_mat'
    prop_values = 'u'
  []
[]

[NodalKernels]
  [rxn]
    type = ReactionNodalKernel
    variable = u
  []
  [ffn]
    type = UserForcingFunctorNodalKernel
    variable = u
    functor = analytic
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
  [dot_err]
    type = ElementL2FunctorError
    approximate = elemental_dot
    exact = dot
  []
  [grad_err]
    type = ElementL2FunctorError
    approximate = elemental_grad
    exact = grad
  []
  [grad_dot_err]
    type = ElementL2FunctorError
    approximate = elemental_grad_dot
    exact = grad_dot
  []
[]

[Outputs]
  csv = true
[]
