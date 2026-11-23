[ActionComponents]
  [cyl]
    type = CylinderComponent
    dimension = 3
    length = 4
    radius = 1
    n_axial = 2
    n_sectors = 2
    block = 'cyl'
  []
  [extrusion]
    type = SurfaceExtrusionComponent
    connected_component = 'cyl'
    source_surface = 'cyl_top_boundary'
    direction = '1 0 0'
    length = 2
    n_axial = 3
  []
[]
