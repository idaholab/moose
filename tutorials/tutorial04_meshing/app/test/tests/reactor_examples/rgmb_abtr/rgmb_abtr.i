[Mesh]
  [rmp]
    type = ReactorMeshParams
    dim = 3
    geom = "Hex"
    assembly_pitch = 14.685
    axial_regions = '50.24 42.32 17.98 16.88 16.88 16.88 16.89 16.88 19.76 65.66 31.14 30.15'
    axial_mesh_intervals = '3 2 1 1 1 1 1 1 1 4 2 2'
    top_boundary_id = 201
    bottom_boundary_id = 202
    radial_boundary_id = 203
  []
  # Define homogenized assembly regions
  [control]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 1
    pitch = 14.685
    region_ids= '12; 9; 4; 4; 4; 4; 4; 5; 6; 6; 7; 15'
    quad_center_elements = true
    homogenized = true
    use_as_assembly = true
  []
  [inner_core]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 14.685
    region_ids= '12; 9; 9; 1; 1; 1; 1; 1; 13; 14; 14; 15'
    quad_center_elements = true
    homogenized = true
    use_as_assembly = true
  []
  [test_fuel]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 3
    pitch = 14.685
    region_ids= '12; 9; 9; 2; 2; 2; 2; 2; 13; 14; 14; 15'
    quad_center_elements = true
    homogenized = true
    use_as_assembly = true
  []
  [inner_reflector]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 4
    pitch = 14.685
    region_ids= '12; 8; 8; 8; 8; 8; 8; 8; 8; 8; 8; 15'
    quad_center_elements = true
    homogenized = true
    use_as_assembly = true
  []
  [outer_core]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 5
    pitch = 14.685
    region_ids= '12; 9; 9; 3; 3; 3; 3; 3; 13; 14; 14; 15'
    quad_center_elements = true
    homogenized = true
    use_as_assembly = true
  []
  [outer_reflector]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 6
    pitch = 14.685
    region_ids= '12; 10; 10; 10; 10; 10; 10; 10; 10; 10; 10; 15'
    quad_center_elements = true
    homogenized = true
    use_as_assembly = true
  []
  [shield]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 7
    pitch = 14.685
    region_ids= '11; 11; 11; 11; 11; 11; 11; 11; 11; 11; 11; 11'
    quad_center_elements = true
    homogenized = true
    use_as_assembly = true
  []
  [core]
    type = CoreMeshGenerator
    inputs = 'control inner_core test_fuel inner_reflector
              outer_core outer_reflector shield dummy'
    dummy_assembly_name = dummy
    pattern =   '  7   7   6   6   6   6   6   6   7   7;
                 7   6   6   5   5   5   5   5   6   6   7;
               6   6   5   5   3   3   3   3   5   5   6   6;
             6   5   5   3   3   3   3   3   3   3   5   5   6;
           6   5   3   3   3   3   4   4   3   3   3   3   5   6;
         6   5   3   3   3   4   4   0   4   4   3   3   3   5   6;
       6   5   3   3   4   4   2   1   1   3   4   4   3   3   5   6;
     6   5   3   3   4   0   1   1   2   1   1   0   4   3   3   5   6;
   7   6   5   3   3   4   1   0   1   1   0   1   4   3   3   5   6   7;
 7   6   5   3   3   4   3   1   1   0   1   1   2   4   3   3   5   6   7;
   7   6   5   3   3   4   1   2   1   1   2   1   4   3   3   5   6   7;
     6   5   3   3   4   0   1   1   0   1   1   0   4   3   3   5   6;
       6   5   3   3   4   4   2   1   1   3   4   4   3   3   5   6;
         6   5   3   3   3   4   4   0   4   4   3   3   3   5   6;
           6   5   3   3   3   3   4   4   3   3   3   3   5   6;
             6   5   5   3   3   3   3   3   3   3   5   5   6;
               6   6   5   5   3   3   3   3   5   5   6   6;
                 7   6   6   5   5   5   5   5   6   6   7;
                   7   7   6   6   6   6   6   6   7   7'
    extrude = true
  []
  [abtr_mesh]
    type = ExtraElementIDCopyGenerator
    input = core
    source_extra_element_id = region_id
    target_extra_element_ids = 'material_id'
  []
[]
