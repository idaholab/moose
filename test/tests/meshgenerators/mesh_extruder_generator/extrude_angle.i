[Mesh]
  [./fmg]
    type = FileMeshGenerator
    file = chimney_quad.e
  []

  [./extrude]
    type = MeshExtruderGenerator
    input = fmg
    num_layers = 20
    extrusion_vector = '1e-2 1e-2 0'
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
  file_base = out_quad_angle
  exodus = true
[]
