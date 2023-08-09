# This input is meant to be combined with parsed_generate_sideset.i

[Mesh]
  # Delete top
  [delete]
    type = BoundaryDeletionGenerator
    input = 'sideset'
    boundary_names = 'top'
  []

  # Recreate it but using the exterior limited feature
  [new_top]
    type = ParsedGenerateSideset
    input = delete
    # all qualify
    combinatorial_geometry = 'z < 1e6'
    # only the old top boundary meets this
    include_external_sides_only = true
    normal = '0 1 0'
    new_sideset_name = top
  []
[]

