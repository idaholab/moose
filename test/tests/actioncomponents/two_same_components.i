[ActionComponents]
  [cylinder_1]
    type = CylinderComponent
    dimension = 2
    radius = 2
    length = 10
    n_axial = 1
    n_radial = 1
    position = '1 0 0'
    direction = '0 1 0'
  []
  [cylinder_2]
    type = CylinderComponent
    dimension = 2
    radius = 4
    length = 1
    n_axial = 1
    n_radial = 1
    position = '0 0 0'
    direction = '1 0 0'
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
