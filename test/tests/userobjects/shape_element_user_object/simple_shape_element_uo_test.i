[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  #nz = 2
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
[]

[Kernels]
  [./diff_u]
    type = Diffusion
    variable = u
  [../]
  [./time_u]
    type = TimeDerivative
    variable = u
  [../]
  [./shape_u]
    type = SimpleTestShapeElementKernel
    user_object = example_uo
    variable = u
  [../]
[]

[UserObjects]
  [./example_uo]
    type = SimpleTestShapeElementUserObject
    u = u
    # as this userobject computes quantities for both the residual AND the jacobian
    # it needs to have these execute_on flags set.
    execute_on = 'linear nonlinear'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options = '-snes_test_display'
  petsc_options_iname = '-snes_type'
  petsc_options_value = 'test'
  dt = 0.1
  num_steps = 2
[]

[Outputs]
  exodus = true
  print_perf_log = true
[]
