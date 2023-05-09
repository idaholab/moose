# Advanced Burner Test Reactor - 3D Core with Homogeneous Assemblies


[Mesh]

  #################################      # This parameter allows us to execute the file but stop at this block so we can see intermediate output.
  final_generator = control              # User: Change this to
  ################################

  # step 1: control
  # step 2: dummy
  # step 3: core
  # step 4: del_dummy
  # step 5: extrude
  # step 6: plane_id
  # step 7: abtr_mesh

  ### Step 1. Define homogeneous assembly types
  # There are 7 unique assembly types in the core. All assemblies of a given type will be considered the same in terms of materials/cross sections.

  [control]
    type = SimpleHexagonGenerator
    hexagon_size = 7.3425 # Half of the assembly pitch, which is 14.685
    hexagon_size_style = 'apothem'   # default
    element_type = QUAD
    block_id = '0'
  []
  [inner_core]
    type = SimpleHexagonGenerator
    hexagon_size = 7.3425
    hexagon_size_style = 'apothem'
    element_type = QUAD
    block_id = '1'
  []
  [test_fuel]
    type = SimpleHexagonGenerator
    hexagon_size = 7.3425
    hexagon_size_style = 'apothem'
    element_type = QUAD
    block_id = '2'
  []
  [inner_reflector]
    type = SimpleHexagonGenerator
    hexagon_size = 7.3425
    hexagon_size_style = 'apothem'
    element_type = QUAD
    block_id = '3'
  []
  [outer_core]
    type = SimpleHexagonGenerator
    hexagon_size = 7.3425
    hexagon_size_style = 'apothem'
    element_type = QUAD
    block_id = '4'
  []
  [outer_reflector]
    type = SimpleHexagonGenerator
    hexagon_size = 7.3425
    hexagon_size_style = 'apothem'
    element_type = QUAD
    block_id = '5'
  []
  [shield]
    type = SimpleHexagonGenerator
    hexagon_size = 7.3425
    hexagon_size_style = 'apothem'
    element_type = QUAD
    block_id = '6'
  []

  ### Step 2. Define dummy assembly type
  ### Dummy assemblies are needed only to make a perfect hex pattern and will be deleted later. Note the assignment of block 997.

  [dummy]
    type = SimpleHexagonGenerator
    hexagon_size = 7.3425
    hexagon_size_style = 'apothem'
    element_type = QUAD
    block_id = '997'
  []


  ### Step 3. Pattern assemblies into a perfect grid by using dummy assemblies to fill in empty slots.

  [core]
    type = PatternedHexMeshGenerator
    inputs = 'control inner_core test_fuel inner_reflector
              outer_core outer_reflector shield dummy'
    pattern_boundary = none           # do not add background coolant or a duct around this pattern
    rotate_angle = 0                  # do not rotate (default is 90 degrees, i.e. vertex up)
    external_boundary_name = radial   # external boundary is called 'radial'
    generate_core_metadata = false    # This is a special case. Even though this is a core, we say "false" since the assemblies
                                      # are homogenized (no pin information) and this is the first invocation of patterning.

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
    id_name = 'assembly_id'         # automatically assigns assembly_ids
    assign_type = cell              # using cell mode
    exclude_id = 'dummy'            # don't assign ids to dummy assemblies
  []

  ### Step 4. Delete dummy assemblies by deleting all elements belonging to block 997.

  [del_dummy]
    type = BlockDeletionGenerator
    input = core
    block = 997                     # delete the elements in block 997 (these are the dummy blocks)
    new_boundary = radial           # rename the newly exposed outer boundary 'radial'
  []


  ### Step 5. Extrude 2D core to 3D.
  # The 2D plane will be extruded upwards into 12 "zones" consisting of varying actual thicknesses and meshing subintervals.

  [extrude]
    type = AdvancedExtruderGenerator
    input = del_dummy
    heights = '50.24 42.32 17.98 16.88 16.88 16.88 16.89 16.88 19.76 65.66 31.14 30.15'
    num_layers = '3 2 1 1 1 1 1 1 1 4 2 2'
    direction = '0 0 1'
    top_boundary = 998
    bottom_boundary = 999

    # This changes the block (subdomain) IDs on each axial layer from the original value (0,1,2,3,4,5,6) to something else
    # There are more than 7 materials in the problem so we introduce new block IDs such as 8,9,10,11,12 to account for different materials.

                      # The first row changes the bottom layer block ids 0 1 2 3 4 5 6 to block ids 12 12 12 12 12 11
    subdomain_swaps = '0 12 1 12 2 12 3 12 4 12 5 12 6 11;
                       0 9  1 9  2 9  3 8  4 9  5 10 6 11;
                       0 4  1 9  2 9  3 8  4 9  5 10 6 11;
                       0 4  1 1  2 2  3 8  4 3  5 10 6 11;
                       0 4  1 1  2 2  3 8  4 3  5 10 6 11;
                       0 4  1 1  2 2  3 8  4 3  5 10 6 11;
                       0 4  1 1  2 2  3 8  4 3  5 10 6 11;
                       0 5  1 1  2 2  3 8  4 3  5 10 6 11;
                       0 6  1 13 2 13 3 8  4 13 5 10 6 11;
                       0 6  1 14 2 14 3 8  4 14 5 10 6 11;
                       0 7  1 14 2 14 3 8  4 14 5 10 6 11;
                       0 15 1 15 2 15 3 15 4 15 5 15 6 11'
                       # The last row changes the block ids on the top layer
  []


  ### Step 6. Add reporting IDs ("plane_id") to each plane
  #  We can later query solution data by plane simply by integrating over elements with the same "plane_id".
  #  The axial heights here should match heights in the mesh.

  [plane_id]
    type = PlaneIDMeshGenerator
    input = extrude
    id_name = plane_id  # add reporting ids called 'plane_id'
    plane_coordinates = '0.000 50.240 92.560 110.540 127.420
                         144.300 161.180 178.070 194.950
                         214.710 280.370 311.510 341.660'  # elements between these coordinates will be labeled with the same plane_id
  []

  ### Step 7. Rename external boundaries to make it easier to reference in Griffin

  [abtr_mesh]
    type = RenameBoundaryGenerator
    input = plane_id
    old_boundary = '999 998'        # The old boundary '999' is renamed 'bottom'.
                                    # The old boundary '998' is renamed 'top'.
    new_boundary = 'bottom top'
  []

  # This is a placeholder in case you want to uniformly refine the mesh later on. It does nothing in this example.
  uniform_refine = 0

[]
