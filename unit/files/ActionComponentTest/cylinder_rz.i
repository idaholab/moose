# A 2D RZ cylinder next to a 2D cylinder that keeps the default XY coordinate system
[ActionComponents]
  [cyl]
    type = CylinderComponent
    dimension = 2
    use_RZ_coordinates = true
    length = 4
    radius = 1
    n_axial = 2
    n_radial = 2
    position = '5 0 0'
    direction = '0 1 0'
    block = 'cyl'
  []
  [cyl_xy]
    type = CylinderComponent
    dimension = 2
    length = 1
    radius = 1
    n_axial = 2
    n_radial = 2
    position = '-5 0 0'
    block = 'cyl_xy'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
