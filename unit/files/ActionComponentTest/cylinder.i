# A 2D cylinder, modified from the command line to trigger each parameter error
[ActionComponents]
  [cyl]
    type = CylinderComponent
    dimension = 2
    length = 4
    radius = 1
    n_axial = 2
    n_radial = 2
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
