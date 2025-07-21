[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    elem_type = HEX8
    nx = 2
    ny = 2
    nz = 2
  []
  [trans]
    type = BoundaryTransitionGenerator
    input = gmg
    boundary_names = 'left'
  []
[]
