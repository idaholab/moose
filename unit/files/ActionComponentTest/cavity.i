# A 2D cylinder enclosed by a cavity, modified from the command line to trigger each error
[Mesh]
  [enclosure]
    type = PolyLineMeshGenerator
    points = '-1 -2 0
              5 -2 0
              5 2 0
              -1 2 0'
    loop = true
  []
[]

[ActionComponents]
  [cyl]
    type = CylinderComponent
    dimension = 2
    length = 4
    radius = 1
    n_axial = 2
    n_radial = 2
  []
  [cavity]
    type = CavityComponent
    dimension = 2
    enclosing_surface_mesh_mg = 'enclosure'
    enclosed_components = 'cyl'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
