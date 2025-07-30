# [Mesh]
#   [circle]
#     type = AnnularMeshGenerator
#     rmax = 1
#     rmin = 0
#     nt = 25
#     nr = 5
#   []
#   [extrude]
#     type = AdvancedExtruderGenerator
#     input = circle
#     direction = '0 0 1'
#     num_layers = '1'
#     heights='1'
#   []
#   [transform]
#     type = TransformGenerator
#     input = extrude 
#     transform = ROTATE_EXT
#     vector_value = '-90 -90 90'
#   []
# []

[ActionComponents]
  [cyl]
    type = CylinderComponent
    dimension = 3
    length = 1
    n_axial = 3
    radius = 1
    n_azimuthal = 25
    n_radial = 5
    direction = '1 0 0'
  []
[]
