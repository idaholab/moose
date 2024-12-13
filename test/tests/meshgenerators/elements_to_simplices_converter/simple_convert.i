[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 2
    nz = 2
  []
  [block_1]
    type = ParsedSubdomainMeshGenerator
    input = gmg
    combinatorial_geometry = 'z>0.5'
    block_id = 1
  []
  [convert]
    type = ElementsToSimplicesConverter
    input = block_1
  []

  # Simplices conversion currently depends on element id numbering
  allow_renumbering = false
[]
