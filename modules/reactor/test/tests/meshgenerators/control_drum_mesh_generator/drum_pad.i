[Mesh]
  [rmp]
    type = ReactorMeshParams
    geom = "Hex"
    assembly_pitch = 20
    flexible_assembly_stitching = true
    radial_boundary_id = 200
    region_id_as_block_name = true
  []
  [drum]
    type = ControlDrumMeshGenerator
    reactor_params = rmp
    assembly_type = 1
    drum_inner_radius = 8
    drum_outer_radius = 9.1
    num_azimuthal_sectors = 180
    drum_inner_intervals = 10

    pad_start_angle = 90
    pad_end_angle = 180
    region_ids = '1 2 3 4'
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
  [area_reg1]
    type = VolumePostprocessor
    block = "RGMB_DRUM1_REG1 RGMB_DRUM1_REG1_TRI"
  []
  [area_reg2]
    type = VolumePostprocessor
    block = "RGMB_DRUM1_REG2"
  []
  [area_reg3]
    type = VolumePostprocessor
    block = "RGMB_DRUM1_REG3"
  []
  [area_reg4]
    type = VolumePostprocessor
    block = "RGMB_DRUM1_REG4_TRI"
  []
[]

[Outputs]
  csv = true
  execute_on = 'FINAL'
[]
