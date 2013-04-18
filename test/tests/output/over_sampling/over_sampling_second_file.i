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
  petsc_options = '-snes_mf_operator'

  dt = 0.2
  start_time = 0
  num_steps = 3
[]

[Output]
  file_base = out_wedge
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true

  [./OverSampling]
    # Here we use the over sampling system to write
    # to a first order mesh for vizualization purposes
    file = wedge6_mesh.e
    exodus = true
    refinements = 0
    output_initial = true
  [../]
[]
