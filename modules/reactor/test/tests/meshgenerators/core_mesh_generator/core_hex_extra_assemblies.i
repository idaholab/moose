[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 3
    geom = "Hex"
    assembly_pitch = 7.10315
    radial_boundary_id = 200
    top_boundary_id = 201
    bottom_boundary_id = 202
    axial_regions = '1.0 1.0'
    axial_mesh_intervals = '1 1'
  []

  [pin1]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 1
    pitch = 1.42063
    num_sectors = 2
    ring_radii = 0.2
    duct_halfpitch = 0.68
    mesh_intervals = '1 1 1'
    quad_center_elements = false
    region_ids = '11 12 13; 111 112 113'
    block_names = 'P1_R11 P1_R12 P1_R13; P1_R111 P1_R112 P1_R113'
  []

  [pin2]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 1.42063
    num_sectors = 2
    quad_center_elements = false
    mesh_intervals = 1
    region_ids = '21; 121'
    block_names = 'P2_R21; P2_R121'
  []

  [pin3]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 3
    pitch = 1.42063
    num_sectors = 2
    ring_radii = '0.3818'
    mesh_intervals = '1 1'
    quad_center_elements = false
    region_ids = '31 32; 131 132'
    block_names = 'P3_R31 P3_R32; P3_R131 P3_R132'
  []

  [amg1]
    type = AssemblyMeshGenerator
    assembly_type = 1
    inputs = 'pin2'
    pattern='  0   0;
             0   0   0;
               0   0'
    background_intervals = 1
    background_region_id = '41 141'
    background_block_name = 'A1_R41 A1_R141'
  []

  [amg2]
    type = AssemblyMeshGenerator
    assembly_type = 2
    inputs = 'pin1 pin3'
    pattern = '0 0;
              0 1 0;
               0 0'
    background_region_id = '51 151'
    background_block_name = 'A2_R51 A2_R151'
    background_intervals = 1
    duct_region_ids = '52; 152'
    duct_block_names = 'A2_R52; A2_R152'
    duct_halfpitch = '3.5'
    duct_intervals = '1'
  []

  [amg3]
    type = AssemblyMeshGenerator
    assembly_type = 3
    inputs = 'pin1 pin3'
    pattern = '0 0;
              0 1 0;
               0 0'
    background_region_id = '51 151'
    background_block_name = 'A2_R51 A2_R151'
    background_intervals = 1
    duct_region_ids = '52; 152'
    duct_block_names = 'A2_R52; A2_R152'
    duct_halfpitch = '3.5'
    duct_intervals = '1'
  []


  [cmg]
    type = CoreMeshGenerator
    inputs = 'amg1 amg2 empty amg3'
    dummy_assembly_name = empty
    pattern = '2 1;
              1 0 2;
               2 1'
    extrude = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Reporters]
  [metadata]
    type = MeshMetaDataReporter
  []
[]

[Outputs]
  file_base = core_hex_extra_assemblies_out
  [json_out]
    type = JSON
    execute_on = FINAL
    execute_system_information_on = none
  []
[]
