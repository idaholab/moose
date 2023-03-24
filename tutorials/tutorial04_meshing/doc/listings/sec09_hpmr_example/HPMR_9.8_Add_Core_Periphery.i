[Mesh]
  [moderator_pincell]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6 # must be six to use hex pattern
    num_sectors_per_side = '2 2 2 2 2 2 '
    background_intervals = 1
    background_block_ids = '10'
    polygon_size = 1.15
    polygon_size_style ='apothem'
    ring_radii = '0.825 0.92'
    ring_intervals = '2 1'
    ring_block_ids = '103 100 101' # 103 is tri mesh
    preserve_volumes = on
    quad_center_elements = false
  []
  [heatpipe_pincell]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6 # must be six to use hex pattern
    num_sectors_per_side = '2 2 2 2 2 2 '
    background_intervals = 1
    background_block_ids = '10'
    polygon_size = 1.15
    polygon_size_style ='apothem'
    ring_radii = '0.97 1.07'
    ring_intervals = '2 1'
    ring_block_ids = '203 200 201' # 203 is tri mesh
    preserve_volumes = on
    quad_center_elements = false
  []
  [fuel_pincell]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6 # must be six to use hex pattern
    num_sectors_per_side = '2 2 2 2 2 2 '
    background_intervals = 1
    background_block_ids = '10'
    polygon_size = 1.15
    polygon_size_style ='apothem'
    ring_radii = '1'
    ring_intervals = '2'
    ring_block_ids = '303 301'  # 303 is tri mesh
    preserve_volumes = on
    quad_center_elements = false
  []
  [fuel_assembly]
    type = PatternedHexMeshGenerator
    inputs = 'fuel_pincell heatpipe_pincell moderator_pincell'
    hexagon_size = 13.376
    background_block_id = 10
    background_intervals = 1
    pattern = '1 0 1 0 1 0 1;
              0 2 0 2 0 2 0 0;
             1 0 1 0 1 0 1 2 1;
            0 2 0 2 0 2 0 0 0 0;
           1 0 1 0 1 0 1 2 1 2 1;
          0 2 0 2 0 2 0 0 0 0 0 0;
         1 0 1 0 1 0 1 2 1 2 1 2 1;
          0 2 0 2 0 2 0 0 0 0 0 0;
           1 0 1 0 1 0 1 2 1 2 1;
            0 2 0 2 0 2 0 0 0 0;
             1 0 1 0 1 0 1 2 1;
              0 2 0 2 0 2 0 0;
               1 0 1 0 1 0 1'
  []
  [cd1_step1]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly fuel_assembly'
    sides_to_adapt = '3 4'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = 504
    ring_radii = '12.25 13.25'
    ring_intervals = '2 1'
    ring_block_ids = '500 501 502'
    preserve_volumes = true
    is_control_drum = true
  []
  [cd1]
    type = AzimuthalBlockSplitGenerator
    input = cd1_step1
    start_angle = 45
    angle_range = 90
    old_blocks = 502
    new_block_ids = 503
  []
  [cd7_step1]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly fuel_assembly'
    sides_to_adapt = '0 1'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = 504
    ring_radii = '12.25 13.25'
    ring_intervals = '2 1'
    ring_block_ids = '500 501 502'
    preserve_volumes = true
    is_control_drum = true
  []
  [cd7]
    type = AzimuthalBlockSplitGenerator
    input = cd7_step1
    start_angle = 225
    angle_range = 90
    old_blocks = 502
    new_block_ids = 503
  []
  [cd10_step1]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly fuel_assembly fuel_assembly'
    sides_to_adapt = '0 4 5'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = 504
    ring_radii = '12.25 13.25'
    ring_intervals = '2 1'
    ring_block_ids = '500 501 502'
    preserve_volumes = true
    is_control_drum = true
  []
  [cd10]
    type = AzimuthalBlockSplitGenerator
    input = cd10_step1
    start_angle = 135
    angle_range = 90
    old_blocks = 502
    new_block_ids = 503
  []
  [cd4_step1]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly fuel_assembly fuel_assembly'
    sides_to_adapt = '1 2 3'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = 504
    ring_radii = '12.25 13.25'
    ring_intervals = '2 1'
    ring_block_ids = '500 501 502'
    preserve_volumes = true
    is_control_drum = true
  []
  [cd4]
    type = AzimuthalBlockSplitGenerator
    input = cd4_step1
    start_angle = 315
    angle_range = 90
    old_blocks = 502
    new_block_ids = 503
  []
  [cd2_step1]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly fuel_assembly fuel_assembly'
    sides_to_adapt = '2 3 4'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = 504
    ring_radii = '12.25 13.25'
    ring_intervals = '2 1'
    ring_block_ids = '500 501 502'
    preserve_volumes = true
    is_control_drum = true
  []
  [cd2]
    type = AzimuthalBlockSplitGenerator
    input = cd2_step1
    start_angle = 15
    angle_range = 90
    old_blocks = 502
    new_block_ids = 503
  []
  [cd3_step1]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly fuel_assembly'
    sides_to_adapt = '2 3'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = 504
    ring_radii = '12.25 13.25'
    ring_intervals = '2 1'
    ring_block_ids = '500 501 502'
    preserve_volumes = true
    is_control_drum = true
  []
  [cd3]
    type = AzimuthalBlockSplitGenerator
    input = cd3_step1
    start_angle = 345
    angle_range = 90
    old_blocks = 502
    new_block_ids = 503
  []
  [cd5_step1]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly fuel_assembly'
    sides_to_adapt = '1 2'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = 504
    ring_radii = '12.25 13.25'
    ring_intervals = '2 1'
    ring_block_ids = '500 501 502'
    preserve_volumes = true
    is_control_drum = true
  []
  [cd5]
    type = AzimuthalBlockSplitGenerator
    input = cd5_step1
    start_angle = 285
    angle_range = 90
    old_blocks = 502
    new_block_ids = 503
  []
  [cd6_step1]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = ' fuel_assembly fuel_assembly fuel_assembly'
    sides_to_adapt = '0 1 2'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = 504
    ring_radii = '12.25 13.25'
    ring_intervals = '2 1'
    ring_block_ids = '500 501 502'
    preserve_volumes = true
    is_control_drum = true
  []
  [cd6]
    type = AzimuthalBlockSplitGenerator
    input = cd6_step1
    start_angle = 255
    angle_range = 90
    old_blocks = 502
    new_block_ids = 503
  []
  [cd8_step1]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = ' fuel_assembly fuel_assembly fuel_assembly'
    sides_to_adapt = '0 1 5'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = 504
    ring_radii = '12.25 13.25'
    ring_intervals = '2 1'
    ring_block_ids = '500 501 502'
    preserve_volumes = true
    is_control_drum = true
  []
  [cd8]
    type = AzimuthalBlockSplitGenerator
    input = cd8_step1
    start_angle = 195
    angle_range = 90
    old_blocks = 502
    new_block_ids = 503
  []
  [cd9_step1]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly fuel_assembly'
    sides_to_adapt = '0 5'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = 504
    ring_radii = '12.25 13.25'
    ring_intervals = '2 1'
    ring_block_ids = '500 501 502'
    preserve_volumes = true
    is_control_drum = true
  []
  [cd9]
    type = AzimuthalBlockSplitGenerator
    input = cd9_step1
    start_angle = 165
    angle_range = 90
    old_blocks = 502
    new_block_ids = 503
  []
  [cd11_step1]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly fuel_assembly'
    sides_to_adapt = '4 5'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = 504
    ring_radii = '12.25 13.25'
    ring_intervals = '2 1'
    ring_block_ids = '500 501 502'
    preserve_volumes = true
    is_control_drum = true
  []
  [cd11]
    type = AzimuthalBlockSplitGenerator
    input = cd11_step1
    start_angle = 105
    angle_range = 90
    old_blocks = 502
    new_block_ids = 503
  []
  [cd12_step1]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly fuel_assembly fuel_assembly'
    sides_to_adapt = '3 4 5'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = 504
    ring_radii = '12.25 13.25'
    ring_intervals = '2 1'
    ring_block_ids = '500 501 502'
    preserve_volumes = true
    is_control_drum = true
  []
  [cd12]
    type = AzimuthalBlockSplitGenerator
    input = cd12_step1
    start_angle = 75
    angle_range = 90
    old_blocks = 502
    new_block_ids = 503
  []
  [refl0]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly'
    sides_to_adapt = '4'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = '400 401'
  []
  [refl1]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly'
    sides_to_adapt = '5'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = '400 401'
  []
  [refl2]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly'
    sides_to_adapt = '0'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = '400 401'
  []
  [refl3]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly'
    sides_to_adapt = '1'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = '400 401'
  []
  [refl4]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly'
    sides_to_adapt = '2'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = '400 401'
  []
  [refl5]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    inputs = 'fuel_assembly'
    sides_to_adapt = '3'
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = '400 401'
  []
  [air_center]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    num_sectors_per_side= '4 4 4 4 4 4'
    inputs = 'fuel_assembly fuel_assembly fuel_assembly fuel_assembly fuel_assembly fuel_assembly'
    sides_to_adapt = '0 1 2 3 4 5'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = '600 601'
  []
  [dummy]
    type =HexagonConcentricCircleAdaptiveBoundaryMeshGenerator
    num_sectors_per_side= '4 4 4 4 4 4'
    hexagon_size = 13.376
    background_intervals = 2
    background_block_ids = '700 701'
       # external_boundary_id = 9998
  []
  [core]
    type = PatternedHexMeshGenerator
    inputs = 'fuel_assembly cd1 cd2 cd3 cd4 cd5 cd6 cd7 cd8 cd9 cd10 cd11 cd12 refl0 refl1 refl2 refl3 refl4 refl5 dummy air_center'
    # Pattern ID  0           1   2   3   4   5   6   7   8   9   10   11   12    13    14    15    16    17    18   19  20
    pattern_boundary = none
    generate_core_metadata = true
    pattern = '19 13 1  18 19;
             13 12  0  0  2 18;
           11  0  0  0  0  0  3;
          14 0  0   0  0  0  0 17;
        19 10  0  0  20  0  0  4 19;
          14 0  0   0  0  0  0 17;
            9  0  0  0  0  0  5;
             15  8  0  0  6 16;
               19 15  7 16 19'
    rotate_angle = 60
  []
  [del_dummy]
    type = BlockDeletionGenerator
    block = '700 701'
    input = core
    new_boundary = 10000
  []
  [outer_shield]
    type = PeripheralRingMeshGenerator
    input = del_dummy
    peripheral_layer_num = 1
    peripheral_ring_radius = 115.0
    input_mesh_external_boundary = 10000
    peripheral_ring_block_id = 250
    peripheral_ring_block_name = outer_shield
  []
 []
