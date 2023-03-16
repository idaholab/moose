# This input file is used to test the Jacobian of an arbitrary ADScalarKernel.
# A test ADScalarKernel is used that uses values from other scalar variables,
# as well as a quantity computed in an elemental user object using a field
# variable.

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 10
[]

[Kernels]
  [time_w]
    type = TimeDerivative
    variable = w
  []
  [diff_w]
    type = Diffusion
    variable = w
  []
[]

[ScalarKernels]
  [time_u]
    type = ADScalarTimeDerivative
    variable = u
  []
  [test_u]
    type = RenameCoupledScalarVarScalarKernel
    variable = u
    coupled_scalar_variable = v
    test_uo = test_uo
  []

  [time_v]
    type = ADScalarTimeDerivative
    variable = v
  []
[]

[UserObjects]
  [test_uo]
    type = TestADScalarKernelUserObject
    variable = w
    execute_on = 'LINEAR NONLINEAR'
  []
[]

[BCs]
  [left]
    type = DirichletBC
    value = 0
    variable = w
    boundary = 'left'
  []
  [right]
    type = DirichletBC
    value = 1
    variable = w
    boundary = 'right'
  []
[]

[Variables]
  [u]
    family = SCALAR
    order = FIRST
    initial_condition = 1.0
  []
  [v]
    family = SCALAR
    order = FIRST
    initial_condition = 3.0
  []
  [w]
    family = LAGRANGE
    order = FIRST
    initial_condition = 3.0
  []
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 1
  solve_type = NEWTON
[]

[Outputs]
  csv = true
[]
