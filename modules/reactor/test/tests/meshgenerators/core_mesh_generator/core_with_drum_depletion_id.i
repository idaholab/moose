[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 3
    geom = "Hex"
    assembly_pitch = 20
    flexible_assembly_stitching = true
    radial_boundary_id = 200
    top_boundary_id = 201
    bottom_boundary_id = 202
    axial_regions = 1.0
    axial_mesh_intervals = 1
    region_id_as_block_name = true
  []
  [het_pin]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 1
    pitch = 1.0
    num_sectors = 2
    ring_radii = '0.4'
    mesh_intervals = '1 1'    # Fuel, background
    region_ids = '1 2'
    quad_center_elements = false
  []
  [het_assembly]
    type = AssemblyMeshGenerator
    assembly_type = 1
    background_intervals = 1
    background_region_id = '3'
    duct_halfpitch = '9'
    duct_intervals = '1'
    duct_region_ids = '4'
    inputs = 'het_pin'
    pattern = '0 0;
              0 0 0;
               0 0'
  []
  [hom_assembly]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 20
    region_ids = '5'
    homogenized = true
    use_as_assembly = true
    quad_center_elements = false
  []
  [drum]
    type = ControlDrumMeshGenerator
    reactor_params = rmp
    assembly_type = 3
    drum_inner_radius = 8
    drum_outer_radius = 9.1
    num_azimuthal_sectors = 36
    drum_inner_intervals = 10
    drum_intervals = 1

    pad_start_angle = 90
    pad_end_angle = 180
    region_ids = '6 7 8 9'
  []
  [core]
    type = CoreMeshGenerator
    inputs = 'het_assembly hom_assembly drum dummy'
    dummy_assembly_name = dummy
    pattern = '
                 1 2;
                3 0 3;
                 2 1'
    extrude = true

    generate_depletion_id = true
    depletion_id_type = pin
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]


[AuxVariables]
  [volume]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 1.0
  []
[]

[VectorPostprocessors]
  [region_volumes]
    type = ExtraIDIntegralVectorPostprocessor
    variable = volume
    id_name = depletion_id
  []
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
