[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 6
    ny = 6
    nz = 0
    zmin = 0
    zmax = 0
    elem_type = QUAD4
  []

  [extrude]
    type = AdvancedExtruderGenerator
    input = gmg
    heights = '6'
    num_layers = '6'
    direction = '0 0 1'
    twist_pitch = 6
  []
[]
