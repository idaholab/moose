# TODO: read the mesh from XDA mesh file

[Mesh]
  [./Generation]
    dim = 2
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    nx = 20
    ny = 20
  [../]
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
  perf_log = true
  petsc_options = '-snes_mf_operator'
  
  restart_soln_file = out_part1_0005.xda
[]

[Output]
  file_base = out_part2
  output_initial = true
  interval = 1
  exodus = true
[]
