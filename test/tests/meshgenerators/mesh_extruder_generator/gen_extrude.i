[Mesh]
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
    bottom_sideset = 'new_front'
    top_sideset = 'new_back'
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
  [./first]
    type = DirichletBC
    variable = u
    boundary = 'new_front'
    value = 0
  [../]

  [./second]
    type = DirichletBC
    variable = u
    boundary = 'new_back'
    value = 1
  [../]
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  exodus = true
[]
