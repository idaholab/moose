[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 3
    geom = "Square"
    assembly_pitch = 7.10315
    axial_regions = '1.0 1.0 1.0'
    axial_mesh_intervals = '2 2 2'
    procedural_region_ids = false
  []

  [pin1]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 1
    pitch = 1.42063
    num_sectors = 6
    ring_radii = '0.2 0.3818'
    duct_halfpitch = '0.68'
    mesh_intervals = '1 2 1 1'

    quad_center_elements = true
  []
  
  [pin2]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 1.42063
    num_sectors = 6
    mesh_intervals = '4'

    quad_center_elements = true
  []
  
  
  [pin3]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 3
    pitch = 1.42063
    num_sectors = 6
    ring_radii = '0.3818'
    mesh_intervals = '2 1'

    quad_center_elements = true
  []

  [amg1]
    type = AssemblyMeshGenerator
    assembly_type = 1
    inputs = 'pin2'
    pattern = '0 0 0 0 0;
               0 0 0 0 0;
               0 0 0 0 0;
               0 0 0 0 0;
               0 0 0 0 0'          
  []
  
  [amg2]
    type = AssemblyMeshGenerator
    assembly_type = 2
    inputs = 'pin3 pin1 pin2'
    pattern = '1 1 1 1 1;
               1 2 2 2 1;
               1 2 0 2 1;
               1 2 2 2 1;
               1 1 1 1 1'          
  []
  
  [cmg]
    type = CoreMeshGenerator
    inputs = 'amg2 amg1 empty'
    dummy_assembly_name = empty
    pattern = '1 1 1 1 1 1;
               1 1 0 0 1 1;
               1 0 0 0 0 1;
               1 0 0 0 0 1;
               1 1 0 0 1 1;
               1 1 1 1 1 1'
    extrude = true               
  []
[]

[Problem]
  solve = false
[]

[Outputs]
  exodus = true
[]
