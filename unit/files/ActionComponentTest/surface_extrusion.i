# A surface extrusion from a cylinder, modified from the command line to trigger each error
[ActionComponents]
  [cyl]
    type = CylinderComponent
    dimension = 2
    length = 4
    radius = 1
    n_axial = 2
    n_radial = 2
  []
  [extrusion]
    type = SurfaceExtrusionComponent
    connected_component = 'cyl'
    source_surface = 'cyl_top_boundary'
    direction = '1 0 0'
    length = 2
    n_axial = 2
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
