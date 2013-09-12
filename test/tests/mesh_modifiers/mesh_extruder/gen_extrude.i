[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
  nz = 0
  zmin = 0
  zmax = 0
  elem_type = QUAD4
[]

[MeshModifiers]
  [./extrude]
    type = MeshExtruder
    num_layers = 6
    extrusion_vector = '1 0 1'
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

  # Preconditioned JFNK (default)
  solve_type = 'PJFNK'
[]

[Output]
  file_base = out_gen
  output_initial = true
  interval = 1
  exodus = true
  print_linear_residuals = true
  perf_log = true
[]
