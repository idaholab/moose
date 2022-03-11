[GlobalParams]
  use_displaced_mesh = true
[]

[Mesh]
  type = GeneratedMesh
  dim = 1
  nx = 2
  parallel_type = replicated
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
  [./diff_v]
    type = Diffusion
    variable = v
  [../]
  [./shape_w]
    type = ExampleShapeElementKernel
    user_object = example_uo
    v = v
    variable = u
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
    #full = true
    off_diag_row =    'u'
    off_diag_column = 'v'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  dt = 0.1
  num_steps = 2
[]

[Outputs]
  exodus = true
  perf_graph = true
[]
