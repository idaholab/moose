[ActionComponents]
  [cyl]
    type = CylinderComponent
    dimension = 2
    length = 5
    start_radius = 1
    end_radius = 2
    n_radial = 2
    n_axial = 4
  []
[]

# [Mesh]
#   [gmg]
#     type = GeneratedMeshGenerator
#     dim = 1
#     nx = 4
#     xmax = 1
#     xmin = -1
#   []
#   [aeg]
#     type = AdvancedExtruderGenerator
#     input = gmg
#     r_final = 3
#     heights = '4'
#     num_layers = '4'
#     direction = '0 1 0'
#     radial_growth_method = LINEAR
#   []
# []