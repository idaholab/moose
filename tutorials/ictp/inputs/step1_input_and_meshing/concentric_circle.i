hole_radius = 0.08e-2     # [m]
pellet_radius = 0.26e-2   # [m]
clad_radius = 0.3e-2      # [m]
water_radius = 0.3475e-2  # [m]

boundary_layer_frac = 0.1
boundary_layer_radius = ${fparse ((water_radius - clad_radius) * boundary_layer_frac + clad_radius)}

[Mesh]
  [concentric_circle]
    type = ConcentricCircleMeshGenerator
    num_sectors = 12
    radii = '${hole_radius} ${pellet_radius} ${clad_radius} ${boundary_layer_radius}'
    has_outer_square = true
    preserve_volumes = false
    rings = '1 3 1 1 3'
    pitch = ${fparse water_radius * 2}
  []
[]
