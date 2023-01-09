[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = hex_in.e
  []
  [gen]
    type = HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 2
    hexagon_size = 5.0
    sides_to_adapt = 0
    meshes_to_adapt_to = 'fmg'
  []
[]
