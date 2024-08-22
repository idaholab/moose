[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 2
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
    drum_intervals = 1

    pad_start_angle = 90
    pad_end_angle = 180
    region_ids = '1 2 3 4'

    generate_depletion_id = true
    depletion_id_type = pin_type
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
  []
[]

[AuxKernels]
  [volume]
    type = ConstantAux
    variable = volume
    value = 1.0
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
