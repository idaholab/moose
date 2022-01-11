[Mesh]
  [./rmp]
    type = ReactorMeshParams
    dim = 3
    geom = "Hex"
    assembly_pitch = 7.10315
    axial_regions = '1.0 1.0 1.0'
    axial_mesh_intervals = '2 2 2'
    procedural_region_ids = false
  []

  [./pin1]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 1
    pitch = 1.42063
    num_sectors = 6
    ring_radii = '0.2 0.3818'
    duct_halfpitch = '0.68'
    mesh_intervals = '1 2 1 1'
    region_ids = '1 2 3 4;
                  11 12 13 14;
                  21 22 23 24'
    quad_center_elements = false
    extrude = true
  []
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]
