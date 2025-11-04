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
    flexible_assembly_stitching = true
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

  [assembly1]
    type = AssemblyMeshGenerator
    assembly_type = 1
    inputs = 'pin1'
    pattern = '0 0;
              0 0 0;
               0 0'
    background_intervals = 1
    background_region_id = 4
    duct_intervals = 1
    duct_halfpitch = 2.2
    duct_region_ids = 5
  []

  [drum_nopad]
    type = ControlDrumMeshGenerator
    reactor_params = rmp
    assembly_type = 3
    drum_inner_radius = 1.0
    drum_outer_radius = 1.5
    num_azimuthal_sectors = 36
    drum_inner_intervals = 10

    region_ids = '6 7 6'
  []

  [drum_pad]
    type = ControlDrumMeshGenerator
    reactor_params = rmp
    assembly_type = 4
    drum_inner_radius = 1.0
    drum_outer_radius = 1.5
    num_azimuthal_sectors = 36
    drum_inner_intervals = 10

    pad_start_angle = 90
    pad_end_angle = 180
    region_ids = '8 9 10 8'
  []

  [rgmb_core]
    type = CoreMeshGenerator
    inputs = 'assembly1 drum_pad drum_nopad'
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

[Outputs]
  [out]
    type = Exodus
    execute_on = timestep_end
    output_extra_element_ids = true
  []
  file_base = core_in
[]
