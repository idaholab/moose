[Mesh]
  type = FileMesh
  file = ellipse_tri.e
[]

[MeshModifiers]
  [./extrude]
    type = MeshExtruder
    num_layers = 20
    extrusion_vector = '0 0 5'
    bottom_sideset = '2'
    top_sideset = '4'
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
  active = 'diff'

  [./diff]
    type = Diffusion
    variable = u
  [../]
[]

[BCs]
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 0
  [../]

  [./top]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Output]
  linear_residuals = true
  file_base = out_tri
  output_initial = true
  interval = 1
  exodus = true
  perf_log = true
[]

[Debug]
  show_actions = true
[]
