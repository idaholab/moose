[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = multiblock.e
  []

  [./extrude]
    type = MeshExtruderGenerator
    input = fmg
    num_layers = 6
    extrusion_vector = '0 0 2'
    bottom_sideset = 'new_bottom'
    top_sideset = 'new_top'

    # Remap layers
    existing_subdomains = '1 2 5'
    layers = '1 3 5'
    new_ids = '10 12 15
               30 32 35
               50 52 55'
  [../]
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
