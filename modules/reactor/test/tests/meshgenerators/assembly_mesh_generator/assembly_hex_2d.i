[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 2
    geom = "Hex"
    assembly_pitch = 3.7884
    radial_boundary_id = 200
  []

  [pin1]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 1
    pitch = 1.3425
    num_sectors = 2
    ring_radii = '0.4404'
    duct_halfpitch = '0.5404'
    mesh_intervals = '1 1 1'
    quad_center_elements = false
    region_ids='1 2 3'
  []

  [amg]
    type = AssemblyMeshGenerator
    assembly_type = 1
    inputs = 'pin1'
    pattern = '0 0;
              0 0 0;
               0 0'
    background_intervals = 1
    background_region_id = 4
    duct_halfpitch = 1.7703
    duct_intervals = 1
    duct_region_ids = 5
    extrude = false
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
