[Mesh]
  type = FileMesh
  # Read in and work with a second order mesh
  file = wedge18_mesh.e
  # If we have an oversample mesh file, we haven not yet implemented
  # synchronization of its partitioning with the problem mesh, so we
  # need to keep the problem mesh replicated.
  parallel_type = replicated
[]

[Functions]
  [./exact_fn]
    type = ParsedFunction
    expression = t*((x*x)+(y*y))
  [../]

  [./forcing_fn]
    type = ParsedFunction
    expression = -4+(x*x+y*y)
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
    type = BodyForce
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

  solve_type = 'PJFNK'

  dt = 0.2
  start_time = 0
  num_steps = 3
[]

[Outputs]
  file_base = out_wedge
  [./oversample]
    type = Exodus
    file_base = out_wedge_oversample
    file = wedge6_mesh.e
  [../]
[]
