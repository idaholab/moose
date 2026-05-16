[ActionComponents]
  combine_component_meshes = false
  [cyl1]
    type = CylinderComponent
    dimension = 1
    length = 1
    n_axial = 4
    radius = 1

    # optional parameters
    position = '0 0 0'
    direction = '1 0 0'
  []
  [cyl2]
    type = CylinderComponent
    dimension = 1
    length = 1
    n_axial = 4
    radius = 1

    # optional parameters
    position = '1 0 0'
    direction = '0 1 0'
  []
  [junction_bottom_right]
    type = JunctionComponent
    first_component = cyl1
    second_component = cyl2
    first_boundary = 'cyl1_right'
    second_boundary = 'cyl2_left'
    junction_method = stitch_meshes
  []
  [cyl3]
    type = CylinderComponent
    dimension = 1
    length = 1
    n_axial = 4
    radius = 1

    # optional parameters
    position = '1 1 0'
    direction = '-1 0 0'
  []
  [junction_top_right]
    type = JunctionComponent
    # for now we have to treat those like mesh generators
    # and write down the name of the last component with the others as inputs
    first_component = junction_bottom_right
    second_component = cyl3
    first_boundary = 'cyl2_right'
    second_boundary = 'cyl3_left'
    junction_method = stitch_meshes
  []
  [cyl4]
    type = CylinderComponent
    dimension = 1
    length = 1
    n_axial = 4
    radius = 1

    # optional parameters
    position = '0 1 0'
    direction = '0 -1 0'
  []
  [junction_top_left]
    type = JunctionComponent
    # for now we have to treat those like mesh generators
    # and write down the name of the last component with the others as inputs
    first_component = junction_top_right
    second_component = cyl4
    first_boundary = 'cyl3_right'
    second_boundary = 'cyl4_left'
    junction_method = stitch_meshes
  []
  [junction_bottom_left]
    type = JunctionComponent
    # for now we have to treat those like mesh generators
    # and write down the name of the last component with the others as inputs
    first_component = junction_top_left
    second_component = junction_top_left
    first_boundary = 'cyl4_right'
    second_boundary = 'cyl1_left'
    junction_method = stitch_meshes
  []
[]
