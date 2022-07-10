[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 2
    geom = "Square"
    assembly_pitch = 7.10315
    #axial_regions = '1.0 1.0'
    axial_mesh_intervals = '1'
    top_boundary_id = 201
    bottom_boundary_id = 202
  []

  [pin1]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 1.42063
    num_sectors = 4
    region_ids='1 2 3 4'

    mesh_intervals = '1 1 1 1'
    quad_center_elements = false
    #extrude = true
  []
[]

[AuxVariables]
  [region_id]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[AuxKernels]
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
