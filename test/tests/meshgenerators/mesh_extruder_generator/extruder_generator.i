[MeshGenerators]
  [./gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 2
  []

  [./extrude]
    type = MeshExtruderGenerator
    input = gmg
    num_layers = 3
    extrusion_vector = '1 0 1'
    bottom_sideset = 'new_bottom'
    top_sideset = 'new_top'
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
