[ActionComponents]
  [cylinder_1]
    type = CylinderComponent
    dimension = 2
    radius = 2
    length = 10
    n_axial = 1
    n_radial = 1
    # note that a 2D RZ cylinder's axis goes through the position
    position = '3 -1 0'
    # points up
    direction = '0 1 0'
    block = 'cyl1'
  []
  [cylinder_2]
    type = CylinderComponent
    dimension = 2
    radius = 3
    length = 6
    n_axial = 1
    n_radial = 1
    position = '0 0 0'
    # we support different RZ axis on a per block basis
    direction = '1 0 0'
    block = 'cyl2'
  []
[]

[Problem]
  skip_nl_system_check = true
[]

[Executioner]
  type = Steady
[]

[AuxVariables]
  [dummy]
  []
[]

[Outputs]
  exodus = true
[]
