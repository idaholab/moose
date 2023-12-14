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
    region_ids = '1 2'
    quad_center_elements = false
    num_sectors = 2
    ring_radii = 0.5404
    mesh_intervals = '1 1'
  []

  [amg1]
    type = AssemblyMeshGenerator
    assembly_type = 1
    inputs = 'pin1'
    pattern = '0 0;
              0 0 0;
               0 0'
    background_intervals = 1
    background_region_id = 3
    duct_halfpitch = 1.7703
    duct_intervals = 1
    duct_region_ids = 4
  []

  [cmg]
    type = CoreMeshGenerator
    inputs = 'amg1'
    dummy_assembly_name = empty
    pattern = '0 0;
              0 0 0;
               0 0'
    extrude = false
    mesh_periphery = true
    periphery_generator = quad_ring
    periphery_region_id = 5
    outer_circle_radius = 7
    periphery_num_layers = 1
    desired_area = 5.0
  []
  [elem_gen]
    type = ElementGenerator
    input = cmg
    nodal_positions = '0 0 0
                       1 0 0
                       1 1 0
                       0 1 0'

    element_connectivity = '0 1 2 3'
    elem_type = "QUAD4"
  []
  data_driven = true
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Reporters/metadata]
  type = MeshMetaDataReporter
[]

[Outputs]
  json = true
[]
