[Mesh]
  file = 1d_3d.e
  # Mixed-dimension meshes don't seem to work with ParallelMesh.  The
  # program hangs, I can't get a useful stack trace when I attach to
  # it.  See also #2130.
  distribution = serial
[]

[Variables]
  active = 'u'

  [./u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./left]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 1
  [../]

  [./bottom]
    type = DirichletBC
    variable = u
    boundary = 100
    value = 0
  [../]

  [./top]
    type = DirichletBC
    variable = u
    boundary = 101
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Output]
  file_base = 1d_3d_out
  output_initial = true
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]
