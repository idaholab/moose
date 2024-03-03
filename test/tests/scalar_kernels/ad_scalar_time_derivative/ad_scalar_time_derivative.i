[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 1
[]

[Variables]
  [u]
    family = SCALAR
    order = FIRST
    initial_condition = 0
  []
[]

[ScalarKernels]
  inactive = 'coupled_dot'
  [time]
    type = ADScalarTimeDerivative
    variable = u
  []
  [coupled_dot]
    type = ADCoupledScalarDot
    variable = u
    v = u
  []
  [source]
    type = ParsedODEKernel
    variable = u
    expression = '-5'
  []
[]

[Executioner]
  type = Transient
  scheme = implicit-euler
  dt = 1.0
  num_steps = 3
  solve_type = NEWTON
  nl_abs_tol = 1e-10
[]

[Outputs]
  csv = true
[]
