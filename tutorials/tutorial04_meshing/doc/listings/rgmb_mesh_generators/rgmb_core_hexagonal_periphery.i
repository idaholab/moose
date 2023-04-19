[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 3
    geom = "Hex"
    assembly_pitch = 4.80315
    axial_regions = '1.0'
    axial_mesh_intervals = '1'
    top_boundary_id = 201
    bottom_boundary_id = 202
    radial_boundary_id = 200
  []

  [pin1]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 1
    pitch = 1.42063
    num_sectors = 2
    ring_radii = 0.2
    duct_halfpitch = 0.58
    mesh_intervals = '1 1 1'
    region_ids='1 2 3'
    quad_center_elements = false
  []

  [pin2]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 1.42063
    num_sectors = 2
    mesh_intervals = 1
    region_ids='4'
    quad_center_elements = false
  []

  [pin3]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 3
    pitch = 1.42063
    num_sectors = 2
    mesh_intervals = '1 1'
    ring_radii = 0.3818
    region_ids='5 6'
    quad_center_elements = false
  []

  [assembly1]
    type = AssemblyMeshGenerator
    assembly_type = 1
    inputs = 'pin1 pin2 pin3'
    pattern = '1 2;
              2 0 1;
               1 2'
    background_intervals = 1
    background_region_id = 7
    duct_intervals = 1
    duct_halfpitch = 2.2
    duct_region_ids = 8

  []

  [assembly2]
    type = AssemblyMeshGenerator
    assembly_type = 2
    inputs = 'pin2'
    pattern = '0 0;
              0 0 0;
               0 0'
    background_intervals = 1
    background_region_id = 9
  []

  [rgmb_core]
    type = CoreMeshGenerator
    inputs = 'assembly1 assembly2 empty'
    dummy_assembly_name = empty
    pattern = '2 1;
              1 0 2;
               2 1'
    extrude = false

    mesh_periphery = true
    periphery_generator = triangle
    periphery_region_id = 100
    outer_circle_radius = 8
    outer_circle_num_segments = 100
    desired_area = 0.5
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = timestep_end
    output_extra_element_ids = true
  []
  file_base = core_in
[]
