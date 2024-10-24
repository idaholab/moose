[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 3
    nz = 2
    xmax = 3
    ymax = 3
    zmax = 3
  []

  [delete]
    type = BoundaryDeletionGenerator
    input = 'gmg'
    boundary_names = 'top right left front back bottom'
  []

  # Recreate it but using the exterior limited feature
  [new_top]
    type = ParsedGenerateNodeset
    input = delete
    expression = 'z > -1'
    include_only_external_nodes = true
    new_nodeset_name = exterior
  []
[]

