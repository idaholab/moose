[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 3
    xmax = 4
    ymax = 4
    zmax = 4
    nx = 4
    ny = 4
    nz = 4
  []

  [band]
    type = ParsedGenerateSideset
    input = gmg
    combinatorial_geometry = 'y > 0.5'
    new_sideset_name = band
    include_only_external_sides = true
    included_subdomains = 0
  []

  # Moves the part of the band inside the window to a new sideset, removing it from the band
  [window]
    type = ParsedGenerateSideset
    input = band
    combinatorial_geometry = '(x - 1)^2 + (z - 2)^2 < 1.5^2'
    new_sideset_name = window
    included_boundaries = band
    replace = true
  []
[]

[Outputs]
  exodus = true
[]
