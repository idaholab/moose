[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 5
  ny = 5
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    [./InitialCondition]
      type = FunctionIC
      function = (x-0.5)^2
    [../]
  [../]
  [./v]
    order = THIRD
    family = HERMITE
    [./InitialCondition]
      type = FunctionIC
      function = (y-0.5)^2
    [../]
  [../]
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
  [./time_u]
    type = TimeDerivative
    variable = u
  [../]
  [./time_v]
    type = TimeDerivative
    variable = v
  [../]
[]

[UserObjects]
  [./test]
    type = TestShapeElementUserObject
    u = u
    # first order lagrange variables have 4 DOFs per element
    u_dofs = 4
    v = v
    # third order hermite variables have 16 DOFs per element
    v_dofs = 16
    # as this userobject computes quantities for both the residual AND the jacobian
    # it needs to have these execute_on flags set.
    execute_on = 'linear nonlinear'
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.1
  num_steps = 2
[]
