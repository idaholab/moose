# Covers the not-watertight log branch in SBMSurfaceMeshBuilder::initialSetup
# by feeding it a deliberately open EDGE2 line (no closed loop).

[Problem]
  solve = false
[]

[Mesh]
  # Open straight line of EDGE2 elements: an obviously non-watertight surface.
  [open_line]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 8
    xmin = 0.0
    xmax = 1.0
    elem_type = EDGE2
    save_with_name = 'open_surface'
  []
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '4'
    dy = '4'
    ix = '8'
    iy = '8'
    subdomain_id = '1'
  []
  add_subdomain_ids = 2
  add_sideset_names = 'SBMinterface'
  final_generator = 'gen'
[]

[UserObjects]
  [TreeBuilder]
    type = SBMSurfaceMeshBuilder
    surface_mesh = open_surface
    check_watertightness = true # exercises the not-watertight log branch
  []
[]

[Executioner]
  type = Steady
[]
