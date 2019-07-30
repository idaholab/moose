[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 6
    ny = 6
    nz = 0
    zmin = 0
    zmax = 0
    elem_type = QUAD4
  []

  [./extrude]
    type = MeshExtruderGenerator
    input = gmg
    num_layers = 6
    extrusion_vector = '1 0 1'
    bottom_sideset = '10'
    top_sideset = '20'
  []
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

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out_gen
  exodus = true
[]
