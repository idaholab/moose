[Mesh]
  #################################      # This parameter allows us to execute the file but stop at this block so we can see intermediate output.
  final_generator = abtr_mesh            # User: Change this to 'pin1', 'assembly', or 'lfr_assy'
                                         # Note: If `extrude = true` is not specified in PinMeshGenerator, a 2-D mesh is created. To visualize
                                         #       the 3-D PinMeshGenerator mesh, `extrude = true` needs to be moved from AssemblyMeshGenerator to
                                         #       PinMeshGenerator, as this parameter can be set to true only once in the input file. All subsequent
                                         #       lines after the PinMeshGenerator block also need to be commented out.
  #################################

  # step 1: lfr_pin
  # step 2: lfr_assembly
  # step 3: lfr_assembly mesh with `material_id` reporting IDs

  ### Step 0. Define global parameters for Reactor Geometry Mesh Builder workflow
              # Note: This step does not produce any mesh
  [rmp]
    type = ReactorMeshParams
    dim = 3                                         # Dimensionality of output mesh (2 or 3)
    geom = "Hex"                                    # Geometry type (Hex or Square)
    assembly_pitch = 16.4165                        # Size of assembly flat-to-flat pitch
    axial_regions = '10.07 30.79 6.56 85.85 1.52
                     106.07 1.51 12.13 5.05 93.87'  # Size of each axial zone
    axial_mesh_intervals = '1 3 1 9 1 20 1 2 1 9'   # Number of subintervals per axial zone
    top_boundary_id = 201                           # Boundary id assigned to top surface
    bottom_boundary_id = 202                        # Boundary id assigned to bottom surface
    radial_boundary_id = 200                        # Boundary id assigned to radial surface
  []

  ### Step 1. Define heterogeneous pin regions
              # Note: There are 7 pin types, each with varying region ID distribution
  [pin1]
    type = PinMeshGenerator
    reactor_params = rmp                       # Name of ReactorMeshParams object
    pin_type = 1                               # Unique identifier for each pin type
    pitch = 1.3425                             # Pin pitch
    num_sectors = 2                            # Number of azimuthal sectors per quadrant
    mesh_intervals = '1 3 1 1 1'               # Number of mesh intervals per radial region
                                               # (4 ring regions followed by background region)
    ring_radii = '0.2020 0.4319 0.4495 0.5404' # Radii of each ring region
    region_ids='1 1 1 1 1;
                2 2 2 2 2;
                3 3 3 3 3;
                4 4 4 5 6;
                8 8 8 9 10;
                20 19 20 13 21;
                24 24 24 25 26;
                28 28 28 29 30;
                32 32 32 32 32;
                33 33 33 33 33'                # Region ID's, assigned radially outwards (4 rings + background) then
                                               # axially from bottom to top
    quad_center_elements = false               # Whether to discretize central ring region into quad or tri elements
  []
  [pin2]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 2
    pitch = 1.3425
    num_sectors = 2
    mesh_intervals = '1 3 1 1 1'
    ring_radii = '0.2020 0.4319 0.4495 0.5404'
    region_ids='1 1 1 1 1;
                2 2 2 2 2;
                3 3 3 3 3;
                4 4 4 5 6;
                8 8 8 9 10;
                20 18 20 13 21;
                24 24 24 25 26;
                28 28 28 29 30;
                32 32 32 32 32;
                33 33 33 33 33'
    quad_center_elements = false
  []
  [pin3]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 3
    pitch = 1.3425
    num_sectors = 2
    mesh_intervals = '1 3 1 1 1'
    ring_radii = '0.2020 0.4319 0.4495 0.5404'
    region_ids='1 1 1 1 1;
                2 2 2 2 2;
                3 3 3 3 3;
                4 4 4 5 6;
                8 8 8 9 10;
                20 17 20 13 21;
                24 24 24 25 26;
                28 28 28 29 30;
                32 32 32 32 32;
                33 33 33 33 33'
    quad_center_elements = false
  []
  [pin4]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 4
    pitch = 1.3425
    num_sectors = 2
    mesh_intervals = '1 3 1 1 1'
    ring_radii = '0.2020 0.4319 0.4495 0.5404'
    region_ids='1 1 1 1 1;
                2 2 2 2 2;
                3 3 3 3 3;
                4 4 4 5 6;
                8 8 8 9 10;
                20 16 20 13 21;
                24 24 24 25 26;
                28 28 28 29 30;
                32 32 32 32 32;
                33 33 33 33 33'
    quad_center_elements = false
  []
  [pin5]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 5
    pitch = 1.3425
    num_sectors = 2
    mesh_intervals = '1 3 1 1 1'
    ring_radii = '0.2020 0.4319 0.4495 0.5404'
    region_ids='1 1 1 1 1;
                2 2 2 2 2;
                3 3 3 3 3;
                4 4 4 5 6;
                8 8 8 9 10;
                20 15 20 13 21;
                24 24 24 25 26;
                28 28 28 29 30;
                32 32 32 32 32;
                33 33 33 33 33'
    quad_center_elements = false
  []
  [pin6]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 6
    pitch = 1.3425
    num_sectors = 2
    mesh_intervals = '1 3 1 1 1'
    ring_radii = '0.2020 0.4319 0.4495 0.5404'
    region_ids='1 1 1 1 1;
                2 2 2 2 2;
                3 3 3 3 3;
                4 4 4 5 6;
                8 8 8 9 10;
                20 14 20 13 21;
                24 24 24 25 26;
                28 28 28 29 30;
                32 32 32 32 32;
                33 33 33 33 33'
    quad_center_elements = false
  []
  [pin7]
    type = PinMeshGenerator
    reactor_params = rmp
    pin_type = 7
    pitch = 1.3425
    num_sectors = 2
    mesh_intervals = '1 3 1 1 1'
    ring_radii = '0.2020 0.4319 0.4495 0.5404'
    region_ids='1 1 1 1 1;
                2 2 2 2 2;
                3 3 3 3 3;
                4 4 4 5 6;
                8 8 8 9 10;
                20 12 20 13 21;
                24 24 24 25 26;
                28 28 28 29 30;
                32 32 32 32 32;
                33 33 33 33 33'
    quad_center_elements = false
  []

  ### Step 2. Pattern heterogeneous pins into hexagonal assembly
  [assembly]
    type = AssemblyMeshGenerator
    inputs = 'pin1 pin2 pin3 pin4 pin5 pin6 pin7'      # Name of constituent pins
    pattern =  ' 0 0 0 0 0 0 0;
                0 1 1 1 1 1 1 0;
               0 1 2 2 2 2 2 1 0;
              0 1 2 3 3 3 3 2 1 0;
             0 1 2 3 4 4 4 3 2 1 0;
            0 1 2 3 4 5 5 4 3 2 1 0;
           0 1 2 3 4 5 6 5 4 3 2 1 0;
            0 1 2 3 4 5 5 4 3 2 1 0;
             0 1 2 3 4 4 4 3 2 1 0;
              0 1 2 3 3 3 3 2 1 0;
               0 1 2 2 2 2 2 1 0;
                0 1 1 1 1 1 1 0;
                 0 0 0 0 0 0 0'                        # Lattice pattern of constituent pins
    extrude = true                                     # Extrude assembly to 3-D
    assembly_type = 1                                  # Unique identifier for each assembly type
    background_region_id = '1 2 3 6 10 21 26 30 32 33' # Region ID's of background region, defined axially from bottom to top
    background_intervals = '1'                         # Number of mesh intervals per background region
    duct_halfpitch = '7.6712 8.0245'                   # Half pitch of each duct region
    duct_intervals = '1 1'                             # Number of mesh intervals per duct region
    duct_region_ids = '1  1;  2  2;  3  3;  7  6;
                      11 10; 22 23; 27 26; 31 30;
                      32 32; 33 33'                    # Region ID's of duct region, assigned radially outwards then axially from bottom to top
  []

  ### Step 3. Copy "region_id" reporting IDs with name "material_id", for use with Griffin reactor physics code
  [lfr_assy]
    type = ExtraElementIDCopyGenerator
    input = assembly
    source_extra_element_id = region_id
    target_extra_element_ids = 'material_id'
  []
[]
