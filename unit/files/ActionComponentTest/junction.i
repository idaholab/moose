# Two 2D cylinders joined by a junction, modified from the command line to trigger each error
[ActionComponents]
  [cyl1]
    type = CylinderComponent
    dimension = 2
    length = 1
    radius = 1
    n_axial = 2
    n_radial = 2
  []
  [cyl2]
    type = CylinderComponent
    dimension = 2
    length = 1
    radius = 1
    n_axial = 2
    n_radial = 2
    position = '2 0 0'
  []
  [junction]
    type = ComponentJunction
    first_component = cyl1
    second_component = cyl2
    first_boundary = 'cyl1_top_boundary'
    second_boundary = 'cyl2_bottom_boundary'
    junction_method = 'stitch_meshes'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
