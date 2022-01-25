[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 3
    geom = "Square"
    assembly_pitch = 7.10315
    axial_regions = '1.0 1.0'
    axial_mesh_intervals = '1 1'
  []

  [pin1]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 1
    pitch = 1.42063
    num_sectors = 2
    ring_radii = '0.2'
    duct_halfpitch = '0.68'
    mesh_intervals = '1 1 1'
    quad_center_elements = false
  []

  [pin2]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 1.42063
    num_sectors = 2
    mesh_intervals = '2'
  []

  [amg]
    type = AssemblyMeshGenerator
    assembly_type = 1
    inputs = 'pin1 pin2'
    pattern = '0 0;
               0 1;'
    extrude = true
  []
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]
