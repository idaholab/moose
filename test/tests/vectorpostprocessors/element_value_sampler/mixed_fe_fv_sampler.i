[Mesh]
  [cmg]
    type = CartesianMeshGenerator
    dim = 1
    dx = 3
    ix = 10
  []
[]

[AuxVariables]
  [T]
    type = MooseVariableFVReal
    [InitialCondition]
      type = FunctionIC
      function = '10 * x*x'
    []

  []
  [grad_T]
    order = CONSTANT
    family = MONOMIAL_VEC
  []
  [auxGrad_T_x]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[AuxKernels]
  [grad_T_aux]
    type = FunctorElementalGradientAux
    variable = grad_T
    functor = T
  []
  [grad_T_x_aux]
    type = VectorVariableComponentAux
    variable = auxGrad_T_x
    vector_variable = grad_T
    component = 'x'
  []
[]

[VectorPostprocessors]
  [element_value_sampler]
    type = ElementValueSampler
    variable = 'T auxGrad_T_x'
    sort_by = id
  []
[]

[Outputs]
  csv = true
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]
