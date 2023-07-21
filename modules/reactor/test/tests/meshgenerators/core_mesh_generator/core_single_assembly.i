[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 3
    geom = "Hex"
    assembly_pitch = 1.42063
    radial_boundary_id = 200
    top_boundary_id = 201
    bottom_boundary_id = 202
    axial_regions = '1.0'
    axial_mesh_intervals = '1'
  []

  [pin1]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 1
    pitch = 1.42063
    region_ids='2'
    quad_center_elements = true
    use_as_assembly = true
    homogenized = true
  []

  [pin2]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 1.42063
    region_ids = '2'
    quad_center_elements = false
    use_as_assembly = true
    homogenized = true
  []

  [cmg]
    type = CoreMeshGenerator
    inputs = 'pin1 pin2 empty'
    dummy_assembly_name = empty
    pattern = '2 1;
              1 0 2;
               2 1'
    extrude = true
  []

  [rotate90]
    type = TransformGenerator
    input = cmg
    transform = ROTATE
    vector_value = '0 0 90'
  []

  [translate]
    type = TransformGenerator
    input = rotate90
    transform = TRANSLATE
    vector_value = '0.710315 -0.710315 0'
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
    extra_element_ids_to_output = 'assembly_id assembly_type_id plane_id pin_type_id region_id'
  []
  file_base = core_in
[]
