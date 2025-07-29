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
#     transform = ROTATE
#     vector_value = '-90 -90 -90'
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
    direction = '0 0 1'
  []
[]

# [ActionComponents]
#   [cylinder_1]
#     type = CylinderComponent
#     dimension = 2
#     radius = 2
#     length = 10
#     n_axial = 4
#     n_radial = 2
#     # position = '4 1 0'
#     direction = '1 0 0'
#   []
# []


# [Mesh]
#   [gmg]
#     type = GeneratedMeshGenerator
#     dim = 2
#     xmax = 4
#     ymin = -1
#     ymax = 1
#   []
#   [transform]
#     type = TransformGenerator
#     input =gmg 
#     transform = ROTATE
#     vector_value = '90 90 90'
#   []
# []
