[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 3
    geom = "Square"
    assembly_pitch = 20
    flexible_assembly_stitching = true
    radial_boundary_id = 200
    top_boundary_id = 201
    bottom_boundary_id = 202
    axial_regions = 1.0
    axial_mesh_intervals = 1
    region_id_as_block_name = true
  []
  [het_pin_1]
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
  [het_assembly_1]
    type = AssemblyMeshGenerator
    assembly_type = 1
    background_intervals = 1
    background_region_id = '3'
    inputs = 'het_pin_1'
    pattern = '0 0;
               0 0'
  []

  [het_pin_2]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 1.0
    num_sectors = 2
    ring_radii = '0.4'
    mesh_intervals = '1 1'    # Fuel, background
    region_ids = '5 6'
    quad_center_elements = false
  []
  [het_assembly_2]
    type = AssemblyMeshGenerator
    assembly_type = 2
    background_intervals = 1
    background_region_id = '7'
    inputs = 'het_pin_2'
    pattern = '0 0 0;
               0 0 0;
               0 0 0'
  []

  [het_pin_3]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 3
    pitch = 1.0
    num_sectors = 4
    ring_radii = '0.4'
    mesh_intervals = '1 1'    # Fuel, background
    region_ids = '9 10'
    quad_center_elements = false
  []
  [het_assembly_3]
    type = AssemblyMeshGenerator
    assembly_type = 3
    background_intervals = 1
    background_region_id = '11'
    inputs = 'het_pin_3'
    pattern = '0 0;
               0 0'
  []
  [hom_assembly]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 4
    pitch = 20
    num_sectors = 2
    mesh_intervals = '1'
    region_ids = '13'
    use_as_assembly = true
    quad_center_elements = false
  []
  [hom_assembly_single_pin]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 5
    pitch = 20
    num_sectors = 2
    ring_radii = '2'
    mesh_intervals = '1 1'    # Fuel, background
    region_ids = '14 15'
    use_as_assembly = true
    quad_center_elements = false
  []
  [core]
    type = CoreMeshGenerator
    inputs = 'het_assembly_1 het_assembly_2 het_assembly_3 hom_assembly hom_assembly_single_pin dummy'
    dummy_assembly_name = dummy
    pattern = '5 1 5;
               3 0 2;
               5 4 5'
    extrude = true
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [area_reg3]
    type = VolumePostprocessor
    block = "RGMB_CORE_REG3_TRI"
  []
  [area_reg7]
    type = VolumePostprocessor
    block = "RGMB_CORE_REG7_TRI"
  []
  [area_reg11]
    type = VolumePostprocessor
    block = "RGMB_CORE_REG11_TRI"
  []
  [area_reg13]
    type = VolumePostprocessor
    block = "RGMB_CORE_REG13_TRI"
  []
  [area_reg15]
    type = VolumePostprocessor
    block = "RGMB_CORE_REG15_TRI"
  []
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
