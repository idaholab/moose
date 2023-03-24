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
  # [heatpipe_pincell]
  #   type = PolygonConcentricCircleMeshGenerator
  #   num_sides = 6 # must be six to use hex pattern
  #   num_sectors_per_side = '2 2 2 2 2 2 '
  #   background_intervals = 1
  #   background_block_ids = '10'
  #   polygon_size = 1.15
  #   polygon_size_style ='apothem'
  #   ring_radii = '0.97 1.07'
  #   ring_intervals = '2 1'
  #   ring_block_ids = '203 200 201' # 203 is tri mesh
  #   preserve_volumes = on
  #   quad_center_elements = false
  # []
  # [fuel_pincell]
  #   type = PolygonConcentricCircleMeshGenerator
  #   num_sides = 6 # must be six to use hex pattern
  #   num_sectors_per_side = '2 2 2 2 2 2 '
  #   background_intervals = 1
  #   background_block_ids = '10'
  #   polygon_size = 1.15
  #   polygon_size_style ='apothem'
  #   ring_radii = '1'
  #   ring_intervals = '2'
  #   ring_block_ids = '303 301'  # 303 is tri mesh
  #   preserve_volumes = on
  #   quad_center_elements = false
  # []
 []
