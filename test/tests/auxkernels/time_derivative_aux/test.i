[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 1
  ymin = 0
  ymax = 1
  nx = 3
  ny = 2
[]

[Functions]
  # These functions have implemented time derivatives
  [some_function]
    type = ParsedFunction
    expression = t*(x+y)
  []
  [some_other_function]
    type = PiecewiseLinear
    x = '0 0.05 0.15 0.25'
    y = '1 2 3 4'
  []
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [time]
    type = TimeDerivative
    variable = u
  []
  [reaction]
    type = Reaction
    variable = u
  []
  [diffusion]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = NeumannBC
    variable = u
    value = 5
    boundary = 'left'
  []
[]

[Materials]
  [material]
    type = GenericFunctorMaterial
    prop_names = 'some_matprop'
    prop_values = 'some_function'
  []
[]

[AuxVariables]
  [variable_derivative]
    family = MONOMIAL
    order = CONSTANT
  []
  inactive = 'variable_derivative_fv'
  [variable_derivative_fv]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  []
  [function_derivative_qp]
    family = MONOMIAL
    order = FIRST
  []
  [function_derivative_elem]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  # Time derivative of a nonlinear variable
  [var_derivative]
    type = TimeDerivativeAux
    variable = variable_derivative
    functor = u
    factor = 10
    execute_on = 'TIMESTEP_END'
  []
  # this places the derivative of a FE variable in a FV one
  # let's output a warning
  inactive = 'var_derivative_to_fv'
  [var_derivative_to_fv]
    type = TimeDerivativeAux
    variable = variable_derivative_fv
    functor = u
  []

  # Time derivative of a function: using the functor system
  # Time derivative of a functor material property is not currently supported
  [function_derivative_quadrature_point]
    type = TimeDerivativeAux
    variable = function_derivative_qp
    functor = 'some_function'
    factor = 2
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [function_derivative_element]
    type = TimeDerivativeAux
    variable = function_derivative_elem
    functor = 'some_other_function'
    factor = 2
    execute_on = 'INITIAL TIMESTEP_END'
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
