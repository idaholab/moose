[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 3
  ny = 3
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
  [./w]
    order = FIRST
    family = LAGRANGE
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
  [./shape_w]
    type = ExampleShapeElementKernel
    user_object = example_uo
    u = u
    v = v
    variable = w
  [../]
  [./time_u]
    type = TimeDerivative
    variable = u
  [../]
  [./time_v]
    type = TimeDerivative
    variable = v
  [../]
  [./time_w]
    type = TimeDerivative
    variable = w
  [../]
[]

[UserObjects]
  [./example_uo]
    type = ExampleShapeElementUserObject
    u = u
    v = v
    # as this userobject computes quantities for both the residual AND the jacobian
    # it needs to have these execute_on flags set.
    execute_on = 'linear nonlinear'
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.1
  num_steps = 2
[]
