[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    elem_type = PRISM6
    nx = 2
    ny = 2
    nz = 2
  []
  [trans]
    type = BoundaryElementConversionGenerator
    input = gmg
    boundary_names = 'left'
  []
[]

