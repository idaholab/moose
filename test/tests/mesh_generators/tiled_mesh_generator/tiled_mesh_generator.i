[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 3
    ny = 3
    nz = 3
  []

  [./tmg]
    type = TiledMeshGenerator
    input = gmg
    x_width = 1
    y_width = 1
    z_width = 1

    left_boundary = left
    right_boundary = right
    top_boundary = top
    bottom_boundary = bottom
    front_boundary = front
    back_boundary = back

    x_tiles = 2
    y_tiles = 1
    z_tiles = 5
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
