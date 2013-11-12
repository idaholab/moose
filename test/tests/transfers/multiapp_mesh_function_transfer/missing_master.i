[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  # The MultiAppMeshFunctionTransfer doesn't work with ParallelMesh.
  # See tosub_master.i and #2145 for more information.
  distribution = serial
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

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[Output]
  linear_residuals = true
  output_initial = true
  exodus = true
  perf_log = true
[]

[MultiApps]
  [./sub]
    positions = '0.9 0.5 0'
    type = TransientMultiApp
    app_type = MooseTestApp
    input_files = tosub_sub.i
    execute_on = timestep
  [../]
[]

[Transfers]
  [./to_sub]
    source_variable = u
    direction = to_multiapp
    variable = transferred_u
    execute_on = timestep
    type = MultiAppMeshFunctionTransfer
    multi_app = sub
    error_on_miss = true
  [../]
  [./elemental_to_sub]
    source_variable = u
    direction = to_multiapp
    variable = elemental_transferred_u
    execute_on = timestep
    type = MultiAppMeshFunctionTransfer
    multi_app = sub
    error_on_miss = true
  [../]
[]

