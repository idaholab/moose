[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    elem_type = HEX8
    nx = 6
    ny = 3
    nz = 3
  []
  [internal_side]
    type = ParsedGenerateSideset
    combinatorial_geometry = 'abs(x-0.5)<0.01'
    new_sideset_name = 'internal'
    input = 'gmg'
    normal = '1 0 0'
  []
  [trans]
    type = BoundaryElementConversionGenerator
    input = internal_side
    boundary_names = 'internal'
    conversion_element_layer_number = 2
  []
[]
