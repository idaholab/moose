[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  ymin = 0
  xmax = 9
  ymax = 9
  nx = 9
  ny = 9
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
  type = Transient
  num_steps = 1
  dt = 1
  solve_type = 'NEWTON'
  print_linear_residuals = true
[]

[Output]
  output_initial = true
  exodus = true
  perf_log = true
[]

[MultiApps]
  [./sub]
    type = TransientMultiApp
    app_type = MooseTestApp
    positions = '1 1 0 5 5 0'
    input_files = fromsub_sub.i
  [../]
[]

[Transfers]
  [./v_nodal_tr]
    type = MultiAppProjectionTransfer
    direction = from_multiapp
    execute_on = timestep_begin
    multi_app = sub
    source_variable = v
    variable = v_nodal
    order = FIRST
    family = LAGRANGE
  [../]
  [./v_elemental_tr]
    type = MultiAppProjectionTransfer
    direction = from_multiapp
    execute_on = timestep_begin
    multi_app = sub
    source_variable = v
    variable = v_elemental
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./x_elemental_tr]
    type = MultiAppProjectionTransfer
    direction = from_multiapp
    multi_app = sub
    source_variable = x
    variable = x_elemental
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./x_nodal_tr]
    type = MultiAppProjectionTransfer
    direction = from_multiapp
    multi_app = sub
    source_variable = x
    variable = x_nodal
    order = FIRST
    family = LAGRANGE
  [../]
[]

