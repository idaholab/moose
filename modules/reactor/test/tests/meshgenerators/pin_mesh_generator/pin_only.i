[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 2
    geom = "Square"
    assembly_pitch = 7.10315
    axial_mesh_intervals = '1'
    top_boundary_id = 201
    bottom_boundary_id = 202
  []

  [pin1]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 1.42063
    region_ids='1 2 3 4'
    quad_center_elements = false
  []
[]

[AuxVariables]
  [pin_type_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [assembly_type_id]
    family = MONOMIAL
    order = CONSTANT
  []
  [region_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
  [pin_type_id]
    type = ExtraElementIDAux
    variable = pin_type_id
    extra_id_name = pin_type_id
  []
  [assembly_type_id]
    type = ExtraElementIDAux
    variable = assembly_type_id
    extra_id_name = assembly_type_id
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

[Outputs]
  exodus = true
  execute_on = timestep_end
[]

[Executioner]
  type = Steady
[]
