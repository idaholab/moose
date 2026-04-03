[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 2
    geom = "Square"
    assembly_pitch = 2.84126
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
    quad_center_elements = false
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
  []

  [amg]
    type = AssemblyMeshGenerator
    assembly_type = 1
    inputs = 'pin1 pin2'
    pattern = '0 0;
               0 1'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  [out]
    type = Exodus
    execute_on = timestep_end
    output_extra_element_ids = true
    extra_element_ids_to_output = 'region_id'
  []
[]
