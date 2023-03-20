[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [left]
    type = DirichletBC
    variable = u
    boundary = left
    value = 0
  []
  [right]
    type = DirichletBC
    variable = u
    boundary = right
    value = 1
  []
[]

[Problem]
  type = SlowProblem
  seconds_to_sleep = 4
  print_during_section = false
  nest_inside_section = false
[]

[Executioner]
  type = Steady
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[MultiApps]
  active = ''
  [subapp]
    type = FullSolveMultiApp
    input_files = 'perf_graph_live_print.i'
    cli_args = "perf_graph_live_print.i"
  []
[]

[Outputs]
  perf_graph_live_time_limit = 1
  [console]
    type = Console
    fit_mode = 80
  []
[]
