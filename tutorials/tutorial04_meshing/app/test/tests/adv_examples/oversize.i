[Mesh]
  [hex_1]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 2
    ring_radii = 4.0
    ring_intervals = 2
    ring_block_ids = '10 15'
    ring_block_names = 'center_tri center'
    background_block_ids = 20
    background_block_names = background
    polygon_size = 5.0
    preserve_volumes = on
  []
  [hex_big]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 2
    ring_radii = 10.0
    ring_intervals = 2
    ring_block_ids = '110 115'
    ring_block_names = 'center2_tri center2'
    background_block_ids = 120
    background_block_names = background2
    polygon_size = 13.0
    preserve_volumes = on
  []
  [bkg_rm]
    type = BlockDeletionGenerator
    input = hex_big
    block = 'background2'
    new_boundary = 'big_pin_ext'
  []
  [hex_dummy]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 4 4 4'
    background_intervals = 2
    background_block_ids = '40 45'
    background_block_names = 'background_dummy_tri background_dummy'
    polygon_size = 5.0
    preserve_volumes = on
  []
  [assm]
    type = PatternedHexMeshGenerator
    inputs = 'hex_1 hex_dummy'
    pattern = '0 0 0 0 0;
              0 0 0 0 0 0;
             0 0 0 0 0 0 0;
            0 0 0 1 1 0 0 0;
           0 0 0 1 1 1 0 0 0;
            0 0 0 1 1 0 0 0;
             0 0 0 0 0 0 0;
              0 0 0 0 0 0;
               0 0 0 0 0'
    background_block_id = 25
    background_block_name = "assem_block"
    hexagon_size = 42
  []
  [blk_del]
    type = BlockDeletionGenerator
    input = assm
    block = 'background_dummy_tri background_dummy'
    new_boundary = 'hole_bdry'
  []
  [xyd]
    type = XYDelaunayGenerator
    boundary = 'blk_del'
    input_boundary_names = 'hole_bdry'
    refine_boundary = false
    desired_area = 1
    output_boundary = 'xyd_ext'
    output_subdomain_name = '100'
    holes = 'bkg_rm'
    stitch_holes = 'true'
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'blk_del xyd'
    clear_stitched_boundary_ids = true
    stitch_boundaries_pairs = 'hole_bdry xyd_ext'
    parallel_type = 'replicated'
  []
[]
