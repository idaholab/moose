[ActionComponents]
  combine_component_meshes = false
  [cyl1]
    type = CylinderComponent
    dimension = 1
    length = 1
    n_axial = 4
    radius = 1

    # optional parameters
    direction = '1 0 0'
    position = '0 0 0'
  []
  [cyl2]
    type = CylinderComponent
    dimension = 1
    length = 1
    n_axial = 4
    radius = 1

    # optional parameters
    direction = '1 0 0'
    position = '1 0 0'
  []
  [junction]
    type = JunctionComponent
    first_component = cyl1
    second_component = cyl2
    first_boundary = 'cyl1_right'
    second_boundary = 'cyl2_left'
    junction_method = stitch_meshes
  []
[]
