[Mesh]
  # Create a coarse mesh representing detector positions
  [coarse_mesh]
    type = CartesianMeshGenerator
    dim = 3
    dx = '4.0'
    dy = '0.025 7.6'
    dz = '0.025 3.6 0.025'
    ix = '2'
    iy = '1 3'
    iz = '1 2 1'
    subdomain_id = '
      0 1
      1 0
      0 1
    '
  []
  [coarse_mesh_move]
    type = TransformGenerator
    input = coarse_mesh
    transform = TRANSLATE
    vector_value = '1.275 0.8 0.8'
  []
  [coarse_mesh_remove]
    type = BlockDeletionGenerator
    input = coarse_mesh_move
    block = '0'
  []
[]

[Positions]
  [detector_positions]
    type = ElementCentroidPositions
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  json = true
[]
