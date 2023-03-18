[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 3
    geom = "Hex"
    assembly_pitch = 7.10315
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
    num_sectors = 2
    ring_radii = '0.2'
    duct_halfpitch = '0.68'
    mesh_intervals = '1 1 1'
    region_ids='1 2 5'

    quad_center_elements = true
  []

  [pin2]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 1.42063
    num_sectors = 2
    mesh_intervals = '2'
    region_ids='2'

    quad_center_elements = true
  []


  [pin3]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 3
    pitch = 1.42063
    num_sectors = 2
    ring_radii = '0.3818'
    mesh_intervals = '1 1'
    region_ids='3 4'

    quad_center_elements = true
  []

  [amg1]
    type = AssemblyMeshGenerator
    assembly_type = 1
    background_intervals = 1
    inputs = 'pin2'
    pattern = '0 0;
              0 0 0;
               0 0'
    background_region_id = 10
  []

  [amg2]
    type = AssemblyMeshGenerator
    assembly_type = 2
    background_intervals = 1
    inputs = 'pin1 pin3'
    pattern = '0 0;
              0 1 0;
               1 0'
    background_region_id = 20
  []

  [cmg]
    type = CoreMeshGenerator
    inputs = 'amg1 amg2 empty'
    dummy_assembly_name = empty
    pattern = '1 1;
              1 0 1;
               1 1'
    extrude = false

    mesh_periphery=true
    periphery_generator=quad_ring
    periphery_region_id=30
    outer_circle_radius=15
    periphery_num_layers=3
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
  [out]
    type = Exodus
    output_extra_element_ids = false
  []
  file_base = core_periphery_prmg_in
[]
