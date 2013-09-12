[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  xmax = 1
  ymin = -1
  ymax = 1
  nx = 20
  ny = 20
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    value = (sin(pi*t))
  [../]

  [./forcing_fn]
    type = ParsedFunction
    value = sin(pi*t)
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
  active = 'diff' #ffn'

  [./ie]
    type = TimeDerivative
    variable = u
  [../]

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
  type = Transient

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  dt = 0.1
  start_time = 0
  num_steps = 20
[]

[Postprocessors]
  [./max_nodal_val]
    type = NodalMaxValue
    variable = u
  [../]
[]

[Output]
  file_base = out_nodal_max
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]
