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
[]

[AuxKernels]
  # Time derivative of a nonlinear variable
  [var_derivative]
    type = SecondTimeDerivativeAux
    variable = variable_derivative
    v = u
    factor = 10
    execute_on = 'TIMESTEP_END'
  []
  # this places the derivative of a FE variable in a FV one
  # let's output a warning
  inactive = 'var_derivative_to_fv'
  [var_derivative_to_fv]
    type = SecondTimeDerivativeAux
    variable = variable_derivative_fv
    v = u
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 2
  l_tol = 1e-10
  [TimeIntegrator]
    type = CentralDifference
  []
[]

[Outputs]
  exodus = true
[]
