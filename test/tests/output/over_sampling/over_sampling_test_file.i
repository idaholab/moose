[Mesh]
  type = FileMesh
  file = square_3x3.e
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    value = t*((x*x)+(y*y))
  [../]

  [./forcing_fn]
    type = ParsedFunction
    value = -4+(x*x+y*y)
  [../]
[]

[Variables]
  active = 'u'

  [./u]
    order = THIRD
    family = HERMITE
  [../]
[]

[Kernels]
  active = 'ie diff ffn'

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
    boundary = '1 2 3 4'
    function = exact_fn
  [../]
[]

[Postprocessors]
  [./dt]
    type = TimestepSize
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'



  dt = 0.2
  start_time = 0
  num_steps = 5
[]

[Output]
  file_base = out_file
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true

  [./OverSampling]
    exodus = true
    refinements = 3
    output_initial = true
  [../]
[]
