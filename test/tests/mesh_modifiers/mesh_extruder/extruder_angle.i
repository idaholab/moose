[Mesh]
  type = FileMesh
  file = chimney_quad.e
[]

[MeshModifiers]
  [./extrude]
    type = MeshExtruder
    num_layers = 20
    extrusion_vector = '1e-2 1e-2 0'
    bottom_sideset = '10'
    top_sideset = '20'
  [../]
[]

[Variables]
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
  [./bottom]
    type = DirichletBC
    variable = u
    boundary = 10
    value = 0
  [../]

  [./top]
    type = DirichletBC
    variable = u
    boundary = 20
    value = 1
  [../]
[]

[Executioner]
  type = Steady
  petsc_options = '-snes_mf_operator'
[]

[Output]
  file_base = out_quad_angle
  output_initial = true
  interval = 1
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]
