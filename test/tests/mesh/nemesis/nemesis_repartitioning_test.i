[Mesh]
  file = cylinder/cylinder.e
  pre_split = true
  # leaving skip_partitioning off lets us exodiff against a gold
  # standard generated with default libMesh settings
  # skip_partitioning = true
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  active = 'left right'

  [./left]
    type = DirichletBC
    variable = u
    boundary = 1
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]
[]

[Postprocessors]
  [./elem_avg]
    type = ElementAverageValue
    variable = u
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'NEWTON'

  [./Adaptivity]
    steps = 1
    refine_fraction = 0.1
    coarsen_fraction = 0.1
    max_h_level = 2
  [../]

  nl_rel_tol = 1e-6
[]

[Outputs]
  file_base = repartitioned
  nemesis = true
[]
