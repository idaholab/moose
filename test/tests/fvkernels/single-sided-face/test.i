[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
[]

[Variables]
  [v]
    type = MooseVariableFVReal
  []
[]

[AuxVariables]
  [aux]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [aux]
    type = FunctorADMatPropElementalAux
    variable = aux
    mat_prop = 'first_value'
  []
[]

[FVKernels]
  [diff]
    type = FVDiffusion
    variable = v
    coeff = coeff
  []
  [first]
    type = FVGradAndDotFunctorFluxKernel
    variable = v
    value_functor = "first_value"
    grad_functor = "first_grad"
    dot_functor = "first_dot"
  []
  [second]
    type = FVGradAndDotFunctorFluxKernel
    variable = v
    value_functor = "second_value"
    grad_functor = "second_grad"
    dot_functor = "second_dot"
  []
  [third]
    type = FVGradAndDotFunctorFluxKernel
    variable = v
    value_functor = "third_value"
    grad_functor = "third_grad"
    dot_functor = "third_dot"
  []
[]

[FVBCs]
  [left]
    type = FVDirichletBC
    variable = v
    boundary = left
    value = 7
  []
  [right]
    type = FVDirichletBC
    variable = v
    boundary = right
    value = 42
  []
[]

[Functions]
  [func]
    type = ADParsedFunction
    value = 'x + t'
  []
[]

[Materials]
  [diff]
    type = ADGenericConstantFunctorMaterial
    prop_names = 'coeff'
    prop_values = '1'
  []
  [first]
    type = FunctorPropFromGradAndDot
    functor = 'func'
    root_functor_prop_name = 'first'
  []
  [second]
    type = FunctorPropFromGradAndDot
    functor = 'aux'
    root_functor_prop_name = 'second'
  []
  [third]
    type = FunctorPropFromGradAndDot
    functor = 'v'
    root_functor_prop_name = 'third'
  []
[]

[Problem]
  kernel_coverage_check = off
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 0.5
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Outputs]
  exodus = true
[]
