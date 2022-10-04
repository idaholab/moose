[Mesh]
  [file]
    type = FileMeshGenerator
    file = ellipse_tri.e
  []

  [extrude]
    type = MeshExtruderGenerator
    input = file
    num_layers = 20
    extrusion_vector = '0 0 5'
    bottom_sideset = '2'
    top_sideset = '4'
  []
[]

[Variables]
  active = 'u'

  [u]
    order = FIRST
    family = LAGRANGE
  []
[]

[Kernels]
  active = 'diff'

  [diff]
    type = Diffusion
    variable = u
  []
[]

[BCs]
  [bottom]
    type = DirichletBC
    variable = u
    boundary = 2
    value = 0
  []

  [top]
    type = DirichletBC
    variable = u
    boundary = 4
    value = 1
  []
[]

[Executioner]
  type = Steady

  solve_type = 'PJFNK'
[]

[Outputs]
  file_base = out_tri
  exodus = true
[]

[Debug]
  show_actions = true
[]
