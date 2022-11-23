[Mesh]
  [file]
    type = FileMeshGenerator
    file = 3d_serrated_multiblock.e
  []
  [extrude]
    type = SideSetExtruderGenerator
    input = file
    sideset = '6'
    extrusion_vector = '1 1 0.5'
    num_layers = 3
  []
[]
