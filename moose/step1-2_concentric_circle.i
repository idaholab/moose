hole_diameter = 0.16e-2    # [m]
pellet_diameter = 0.52e-2  # [m]
clad_diameter = 0.6e-2     # [m]
pin_pitch = 0.695e-2       # [m]

boundary_layer_frac = 0.1
boundary_layer_radius = ${fparse ((pin_pitch - clad_diameter) * boundary_layer_frac + clad_diameter) / 2}

[Mesh]
  [concentric_circle]
    type = ConcentricCircleMeshGenerator
    num_sectors = 12
    radii = '${fparse hole_diameter/2} ${fparse pellet_diameter/2} ${fparse clad_diameter/2} ${boundary_layer_radius}'
    has_outer_square = true
    preserve_volumes = false
    rings = '1 3 1 1 3'
    pitch = ${pin_pitch}
  []
[]
