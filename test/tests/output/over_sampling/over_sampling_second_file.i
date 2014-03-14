[Mesh]
  type = FileMesh
  file = wedge18_mesh.e
  # Read in and work with a second order mesh
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
    boundary = '1 2 4'
    function = exact_fn
  [../]
[]

[Executioner]
  type = Transient

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  dt = 0.2
  start_time = 0
  num_steps = 3
[]

[Outputs]
  file_base = out_wedge
  output_initial = true
  [./exodus]
    type = Exodus
    oversample = true
    append_oversample = true
    file = wedge6_mesh.e # Oversample to another mesh file
  [../]
  [./console]
    type = Console
    perf_log = true
  [../]
[]
