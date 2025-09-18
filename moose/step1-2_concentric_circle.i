!include common.i

[Mesh]
  [concentric_circle]
    type = ConcentricCircleMeshGenerator
    num_sectors = 4
    radii = '${hole_diameter} ${pellet_diameter} ${clad_diameter}'
    has_outer_square = false
    preserve_volumes = true
    rings = '1 3 1'
  []
[]
