[Mesh]
  [fmg]
    type = FileMeshGenerator
    file = 'gold/err_internal_in.e'
  []
  [trans]
    type = BoundaryElementConversionGenerator
    input = fmg
    boundary_names = 'new'
    external_boundaries_checking = true
  []
[]
