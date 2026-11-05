[ActionComponents]
  combine_component_meshes = false
  [cyl1]
    type = CylinderComponent
    dimension = 2
    use_RZ_coordinates = true
    block = 'cyl1'
    length = 1
    n_axial = 4
    radius = 1

    # optional parameters
    n_radial = 2
    direction = '0 1 0'
    position = '0 0 0'
  []
  [cyl2]
    type = CylinderComponent
    dimension = 2
    use_RZ_coordinates = true
    block = 'cyl2'
    length = 1
    n_axial = 4
    radius = 1

    # optional parameters
    n_radial = 2
    direction = '1 0 0'
    position = '4 3 0'
  []
  [junction]
    type = ComponentJunction
    first_component = cyl1
    second_component = cyl2
    first_boundary = 'cyl1_top_boundary'
    second_boundary = 'cyl2_bottom_boundary'
    n_elem_normal = 6
  []
[]
