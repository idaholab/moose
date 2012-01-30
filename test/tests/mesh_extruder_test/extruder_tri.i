[Mesh]
  type = MeshExtruder
  file = ellipse_tri.e
  num_layers = 20
  height = 5
  extrusion_axis = 2 # Z
  bottom_sidesets = '2'
  top_sidesets = '4'
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
    boundary = 2
    value = 0
  [../]

  [./right]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 1
  [../]
[]

[Materials]
  active = empty

  [./empty]
    type = EmptyMaterial
    block = 1
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out_tri
  output_initial = true
  interval = 1
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]
