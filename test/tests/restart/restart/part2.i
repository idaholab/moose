[Mesh]
  file = out_part1_cp/0005_mesh.cpr
  distribution = serial
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    value = ((x*x)+(y*y))
  [../]

  [./forcing_fn]
    type = ParsedFunction
    value = -4
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff ffn'

  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = UserForcingFunction
    variable = u
    function = forcing_fn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
  restart_file_base = out_part1_cp/0005
[]

[Outputs]
  file_base = out_part2
  exodus = true
  [./console]
    type = Console
    perf_log = true
  [../]
[]
