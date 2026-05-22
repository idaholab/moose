[ActionComponents]
  combine_component_meshes = false
  [cyl1]
    type = CylinderComponent
    dimension = 1
    length = 1
    n_axial = 4
    radius = 1

    # optional parameters
    block = 'cyl1'
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
    block = 'cyl2'
    position = '1 0 0'
    direction = '0 1 0'
  []
  [junction_bottom_right]
    type = ComponentJunction
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
    block = 'cyl3'
    position = '1 1 0'
    direction = '-1 0 0'
  []
  [junction_top_right]
    type = ComponentJunction
    # for now we have to treat those like mesh generators
    # and write down the name of the last component with the others as inputs
    first_component = cyl2
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
    block = 'cyl4'
    position = '0 1 0'
    direction = '0 -1 0'
  []
  [junction_top_left]
    type = ComponentJunction
    # for now we have to treat those like mesh generators
    # and write down the name of the last component with the others as inputs
    first_component = cyl3
    second_component = cyl4
    first_boundary = 'cyl3_right'
    second_boundary = 'cyl4_left'
    junction_method = stitch_meshes
  []
  [junction_bottom_left]
    type = ComponentJunction
    # for now we have to treat those like mesh generators
    # and write down the name of the last component with the others as inputs
    first_component = cyl4
    second_component = cyl1
    first_boundary = 'cyl4_right'
    second_boundary = 'cyl1_left'
    junction_method = stitch_meshes
  []
[]
