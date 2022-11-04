[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[Functions]
  [./test_function]
    type = ParsedFunction
    expression = '2.5*x^2 + 0.75*y^2 + 0.15*x*y'
  [../]
[]

[AuxVariables]
  [./from_sub]
    family = monomial
    order = first
  [../]
  [./test_var]
    family = monomial
    order = first
    [./InitialCondition]
      type = FunctionIC
      function = test_function
    [../]
  [../]
[]

[Variables]
  [./u]
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  [../]
  [./right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
[]

[Outputs]
  execute_on = 'timestep_end'
  exodus = true
[]

[MultiApps]
  [./sub]
    type = FullSolveMultiApp
    app_type = MooseTestApp
    execute_on = initial
    positions = '0.0 0.0 0.0'
    input_files = high_order_sub.i
  [../]
[]

[Transfers]
  [./from]
    type = MultiAppProjectionTransfer
    execute_on = same_as_multiapp
    from_multi_app = sub
    source_variable = test_var
    variable = from_sub
  [../]
  [./to]
    type = MultiAppProjectionTransfer
    execute_on = same_as_multiapp
    to_multi_app = sub
    source_variable = test_var
    variable = from_parent
  [../]
[]

