[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  # The Transfer system doesn't work quite right with ParallelMesh enabled.
  # Form more information, see #2126
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

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options = '-ksp_monitor'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
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
    execute_on = timestep
    positions = '0.48 0 0'
    input_files = tosub_sub.i
  [../]
[]

[Transfers]
  [./to_sub]
    type = MultiAppNearestNodeTransfer
    direction = to_multiapp
    execute_on = timestep
    multi_app = sub
    source_variable = u
    variable = from_master
  [../]
  [./elemental_to_sub]
    type = MultiAppNearestNodeTransfer
    direction = to_multiapp
    execute_on = timestep
    multi_app = sub
    source_variable = u
    variable = elemental_from_master
  [../]
[]

