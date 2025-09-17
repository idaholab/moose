hole_diameter = 0.16
pellet_diameter = 0.520
clad_diameter = 0.6

[Mesh]
  [concentric_circle]
    type = ConcentricCircleMeshGenerator
    num_sectors = 4
    radii = '${hole_diameter} ${pellet_diameter} ${clad_diameter}'
    has_outer_square = false
    preserve_volumes = true
    rings = '1 3 1'
  []
  [delete_hole]
    type = BlockDeletionGenerator
    input = concentric_circle
    block = 1
    new_boundary = inner
  []
[]
