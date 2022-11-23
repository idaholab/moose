[Mesh]
  [cd]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '4 4 4 8 4 4'
    background_intervals = 1
    ring_radii = '4.2 4.8'
    ring_intervals = '2 1'
    ring_block_ids = '10 15 20'
    ring_block_names = 'center_tri center cd_ring'
    background_block_ids = 30
    background_block_names = background
    polygon_size = 5.0
    preserve_volumes = true
  []
  [cd_azi_define]
    type = AzimuthalBlockSplitGenerator
    input = cd
    start_angle = 280
    angle_range = 100
    old_blocks = '10 15 20'
    new_block_ids = '100 150 200'
    new_block_names = 'center_tri_new center_new cd_ring_new'
    preserve_volumes = true
  []
[]
