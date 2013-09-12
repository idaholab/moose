[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
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
  print_linear_residuals = true

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
    positions = '.1 .1 0 0.6 0.6 0 0.6 0.1 0'
    type = TransientMultiApp
    app_type = MooseTestApp
    input_files = tosub_sub.i
    output_base = tosub_multi_out
    execute_on = timestep
  [../]
[]

[Transfers]
  [./to_sub]
    source_variable = u
    direction = to_multiapp
    variable = transferred_u
    execute_on = timestep
    type = MultiAppDTKInterpolationTransfer
    multi_app = sub
  [../]
[]

