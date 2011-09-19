[Mesh]
  [./Generation]
    dim = 2
    xmin = -1
    xmax = 1
    ymin = -1
    ymax = 1
    nx = 5
    ny = 5
    elem_type = QUAD9
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = SECOND
    family = LAGRANGE
  [../]
[]

[Functions]
  [./forcing_fn]
    type = ParsedFunction
    value = -4
  [../]

  [./exact_fn]
    type = ParsedFunction
    value = ((x*x)+(y*y))
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
  active = 'all'

  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = exact_fn
  [../]
[]

[Postprocessors]
  [./l2_err]
    type = ElementL2Error
    variable = u
    function = exact_fn
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out_steady
  output_initial = false
  interval = 1
  exodus = true
  perf_log = true
[]

