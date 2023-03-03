[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 6
  ny = 6
[]

[Variables]
  [u]
    type = MooseVariableFVReal
    initial_condition = 2
  []
[]

[FVKernels]
  [time]
    type = FVTimeKernel
    variable = u
  []
  [reaction]
    type = FVReaction
    variable = u
    rate = 2.0
  []
  [diffusion]
    type = FVDiffusion
    variable = u
    coeff = 0.1
  []
[]

[FVBCs]
  [left]
    type = FVNeumannBC
    variable = u
    value = 5
    boundary = 'left'
  []
[]

[AuxVariables]
  inactive = 'variable_derivative'
  [variable_derivative]
    family = MONOMIAL
    order = CONSTANT
  []
  [variable_derivative_fv]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
[]

[AuxKernels]
  # Time derivative of a FV variable using the functor system
  [function_derivative_element]
    type = TimeDerivativeAux
    variable = variable_derivative_fv
    functor = 'u'
    factor = 2
  []
  # this places the derivative of a FV variable in a FE one
  # let's output a warning
  inactive = 'function_derivative_element_fv_fe'
  [function_derivative_element_fv_fe]
    type = TimeDerivativeAux
    variable = variable_derivative
    functor = 'u'
    factor = 2
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 2
  nl_abs_tol = 1e-12
[]

[Outputs]
  exodus = true
[]
