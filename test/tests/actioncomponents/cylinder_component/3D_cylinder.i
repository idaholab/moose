# [ActionComponents]
#   [cyl]
#     type = CylinderComponent
#     dimension = 3
#     length = 1
#     n_axial = 3
#     radius = 1
#     n_azimuthal = 25
#     n_radial = 5
#     # direction = '1 0 0'
#   []
# []

[Mesh]
  [ccg]
    type = ConcentricCircleMeshGenerator
    has_outer_square = false
    num_sectors = 4
    preserve_volumes = true
    radii = 1
    rings = 1
  []
[]
