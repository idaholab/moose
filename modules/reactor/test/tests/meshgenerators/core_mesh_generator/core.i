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
    region_ids='1 2 5'
    quad_center_elements = true
  []

  [pin2]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 1.42063
    region_ids='2'
    quad_center_elements = true
  []

  [pin3]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 3
    pitch = 1.42063
    region_ids='3 4'
    quad_center_elements = true
  []

  [amg1]
    type = AssemblyMeshGenerator
    assembly_type = 1
    inputs = 'pin2'
    pattern = '0 0;
               0 0'
  []

  [amg2]
    type = AssemblyMeshGenerator
    assembly_type = 2
    inputs = 'pin3 pin1 pin2'
    pattern = '0 1;
               1 2'
  []

  [cmg]
    type = CoreMeshGenerator
    inputs = 'amg2 amg1 empty'
    dummy_assembly_name = empty
    pattern = '1 0;
               0 1'
    extrude = true
  []
[]

[AuxVariables]
  [assembly_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [assembly_type_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [plane_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [pin_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [pin_type_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [region_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [assembly_id]
    type = ExtraElementIDAux
    variable = assembly_id
    extra_id_name = assembly_id
  []
  [assembly_type_id]
    type = ExtraElementIDAux
    variable = assembly_type_id
    extra_id_name = assembly_type_id
  []
  [plane_id]
    type = ExtraElementIDAux
    variable = plane_id
    extra_id_name = plane_id
  []
  [pin_id]
    type = ExtraElementIDAux
    variable = pin_id
    extra_id_name = pin_id
  []
  [pin_type_id]
    type = ExtraElementIDAux
    variable = pin_type_id
    extra_id_name = pin_type_id
  []
  [region_id]
    type = ExtraElementIDAux
    variable = region_id
    extra_id_name = region_id
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  exodus = true
  file_base = core_in
  execute_on = timestep_end
[]
