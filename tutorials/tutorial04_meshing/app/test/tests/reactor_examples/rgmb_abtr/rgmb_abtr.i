[Mesh]

  #################################      # This parameter allows us to execute the file but stop at this block so we can see intermediate output.
  final_generator = abtr_mesh            # User: Change this to 'control', 'core', or 'abtr_mesh'
                                         # Note: If `extrude = true` is not specified in PinMeshGenerator, a 2-D mesh is created. To visualize
                                         #       the 3-D PinMeshGenerator mesh, `extrude = true` needs to be moved from CoreMeshGenerator to
                                         #       PinMeshGenerator, as this parameter can be set to true only once in the input file. All subsequent
                                         #       lines after the PinMeshGenerator block also need to be commented out.
  #################################

  # step 1: control
  # step 2: core
  # step 3: abtr_mesh

  ### Step 0. Define global parameters for Reactor Geometry Mesh Builder workflow
              # Note: This step does not produce any mesh
  [rmp]
    type = ReactorMeshParams
    dim = 3                                          # Dimensionality of output mesh (2 or 3)
    geom = "Hex"                                     # Geometry type (Hex or Square)
    assembly_pitch = 14.685                          # Size of assembly flat-to-flat pitch
    axial_regions = '50.24 42.32 17.98 16.88
                     16.88 16.88 16.89 16.88
                     19.76 65.66 31.14 30.15'        # Size of each axial zone
    axial_mesh_intervals = '3 2 1 1 1 1 1 1 1 4 2 2' # Number of subintervals per axial zone
    top_boundary_id = 201                            # Boundary id assigned to top surface
    bottom_boundary_id = 202                         # Boundary id assigned to bottom surface
    radial_boundary_id = 203                         # Boundary id assigned to radial surface
  []

  ### Step 1. Define homogenized assembly regions
  [control]
    type = PinMeshGenerator
    reactor_params = rmp        # Name of ReactorMeshParams object
    pin_type = 1                # Unique identifier for each homogenized assembly type
    pitch = 14.685              # Assembly pitch
    region_ids= '12; 9; 4; 4;
                  4; 4; 4; 5;
                  6; 6; 7; 15'  # Region ID's, assigned radially outwards then axially from bottom to top
    quad_center_elements = true # Discretize homogenized assemblies into 2 quadrilaterals
    homogenized = true          # Use SimpleHexagonGenerator to define homogenized assemblies
    use_as_assembly = true      # Treat mesh as assembly for direct stitching into CoreMeshGenerator
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

  ### Step 2. Pattern homogenized assemblies into hexagonal grid with dummy assemblies deleted
  [core]
    type = CoreMeshGenerator
    inputs = 'control inner_core test_fuel inner_reflector
              outer_core outer_reflector shield dummy'    # Name of constituent assemblies
    dummy_assembly_name = dummy                           # Name of dummy assembly, does not need to be explicitly defined
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
                   7   7   6   6   6   6   6   6   7   7' # Lattice pattern of constituent assemblies
    extrude = true                                        # Extrude core to 3-D
  []

  ### Step 3. Copy "region_id" reporting IDs with name "material_id", for use with Griffin reactor physics code
  [abtr_mesh]
    type = ExtraElementIDCopyGenerator
    input = core
    source_extra_element_id = region_id
    target_extra_element_ids = 'material_id'
  []
[]
