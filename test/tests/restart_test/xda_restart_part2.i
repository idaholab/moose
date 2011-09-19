# Use the exodus file for restarting the problem:
# - restart one variable
# - and have one extra variable
# - have PBP active to have more system in Equation system
#

[Mesh]
  file = out_xda_restart_part1_0005_mesh.xda
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
  active = 'u v'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = UserForcingFunction
    variable = u
    function = forcing_fn
  [../]

  [./diff_v]
    type = Diffusion
    variable = v
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]

  [./left_v]
    type = DirichletBC
    variable = v
    boundary = '3'
    value = 0
  [../]

  [./right_v]
    type = DirichletBC
    variable = v
    boundary = '1'
    value = 1
  [../]
[]

[Preconditioning]
  [./PBP]
    type = PBP
    solve_order = 'u v'
    preconditioner = 'amg amg'
  [../]
[]

[Executioner]
  type = Steady
  perf_log = true
#  petsc_options = '-snes_mf'
  restart_soln_file = out_xda_restart_part1_0005.xda
[]

[Output]
  file_base = out_xda_restart_part2
  output_initial = true
  interval = 1
  exodus = true
[]
