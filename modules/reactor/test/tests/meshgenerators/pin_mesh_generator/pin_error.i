[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 3
    geom = "Hex"
    assembly_pitch = 10.42063
    radial_boundary_id = 200
    top_boundary_id = 201
    bottom_boundary_id = 202
    axial_regions = '1.0'
    axial_mesh_intervals = '1'
    flexible_assembly_stitching = true
  []
  [pmg]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    ring_radii = '0.5 0.9'
    pitch = 1.42063
    region_ids = '5 3 4'
    quad_center_elements = false
    num_sectors = 8
    mesh_intervals = '1 1 1'
  []
[]
