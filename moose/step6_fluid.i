hole_diameter = 0.08e-2    # [m]
pellet_diameter = 0.26e-2  # [m]
clad_diameter = 0.3e-2     # [m]
pin_pitch = 0.695e-2       # [m]

[Mesh]
  [concentric_circle]
    type = ConcentricCircleMeshGenerator
    num_sectors = 4
    radii = '${hole_diameter} ${pellet_diameter} ${clad_diameter}'
    has_outer_square = true
    preserve_volumes = true
    rings = '1 3 1 1'
    pitch = ${pin_pitch}
  []
  [delete_fuel_pin]
    type = BlockDeletionGenerator
    input = concentric_circle
    block = '1 2 3'
  []
[]
