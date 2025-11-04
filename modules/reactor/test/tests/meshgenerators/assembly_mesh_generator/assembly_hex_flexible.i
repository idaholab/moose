[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 2
    geom = "Hex"
    assembly_pitch = 7.10315
    flexible_assembly_stitching = true
    region_id_as_block_name = true
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
    quad_center_elements = true
    region_ids='1 2 5'
  []

  [pin2]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 1.42063
    num_sectors = 2
    mesh_intervals = '2'
    region_ids='3'
    quad_center_elements = true
  []

  [amg]
    type = AssemblyMeshGenerator
    assembly_type = 1
    inputs = 'pin1 pin2'
    pattern = '1 1;
              1 0 1;
               1 1'
    background_intervals = 1
    background_region_id = '6'
    duct_halfpitch = '3.5'
    duct_region_ids = '7'
    duct_intervals = '1'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Postprocessors]
  [area_reg1]
    type = VolumePostprocessor
    block = "RGMB_ASSEMBLY1_REG1"
  []
  [area_reg2]
    type = VolumePostprocessor
    block = "RGMB_ASSEMBLY1_REG2"
  []
  [area_reg3]
    type = VolumePostprocessor
    block = "RGMB_ASSEMBLY1_REG3"
  []
  [area_reg5]
    type = VolumePostprocessor
    block = "RGMB_ASSEMBLY1_REG5"
  []
  [area_reg6]
    type = VolumePostprocessor
    block = "RGMB_ASSEMBLY1_REG6"
  []
  [area_reg7]
    type = VolumePostprocessor
    block = "RGMB_ASSEMBLY1_REG7_TRI"
  []
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
