[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 3
    geom = "Square"
    assembly_pitch = 2.84126
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
    region_ids='1 2 5'
    quad_center_elements = true
  []

  [pin2]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 1.42063
    num_sectors = 2
    mesh_intervals = '2'
    region_ids='2'
    quad_center_elements = true
  []

  [pin3]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 3
    pitch = 1.42063
    num_sectors = 2
    ring_radii = 0.3818
    mesh_intervals = '1 1'
    region_ids='3 4'
    quad_center_elements = true
  []

  [assembly1]
    type = AssemblyMeshGenerator
    assembly_type = 2
    inputs = 'pin3 pin1 pin2'
    pattern = '0 1;
               1 2'
  []

  [assembly2]
    type = AssemblyMeshGenerator
    assembly_type = 1
    inputs = 'pin2'
    pattern = '0 0;
               0 0'
  []

  [rgmb_core]
    type = CoreMeshGenerator
    inputs = 'assembly1 assembly2'
    pattern = '1 0;
               0 1'
    extrude = true
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
