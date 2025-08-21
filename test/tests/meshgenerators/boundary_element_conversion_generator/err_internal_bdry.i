[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    elem_type = HEX8
    nx = 2
    ny = 2
    nz = 2
  []
  [int_bdry]
    type = ParsedGenerateSideset
    input = gmg
    new_sideset_name = 'new'
    combinatorial_geometry = 'abs(x-0.5)<0.01'
    normal = '1 0 0'
  []
  [trans]
    type = BoundaryElementConversionGenerator
    input = int_bdry
    boundary_names = 'new'
    external_boundaries_checking = true
  []
[]
