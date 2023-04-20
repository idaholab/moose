[Mesh]
  [FUEL]
    type = PolygonConcentricCircleMeshGenerator
    num_sides = 6
    num_sectors_per_side = '2 2 2 2 2 2'
    background_intervals = 1
    background_block_ids = '4'
    polygon_size = 1.075
    polygon_size_style ='apothem'
    ring_radii = '0.925 0.975'
    ring_intervals = '3 1'
    ring_block_ids = '11 2 3'
    preserve_volumes = on
    quad_center_elements = false
  []

  [ASSEMBLY]
      type = PatternedHexMeshGenerator
      inputs = 'FUEL'
      background_intervals = 1
      background_block_id = '5'
      duct_sizes ='5.0 5.2'
      duct_sizes_style = 'apothem'
      duct_block_ids = '6 7'
      duct_intervals = ' 2 2'
      hexagon_size = '5.5'
      pattern ='0 0 0;
               0 0 0 0;
              0 0 0 0 0;
               0 0 0 0;
                0 0 0'
      assign_type = 'cell'
      id_name = pin_id
  []

  [CORE_2D]
      type = PatternedHexMeshGenerator
      inputs = 'ASSEMBLY'
      generate_core_metadata = true
      pattern_boundary = none
      pattern ='0 0 0;
               0 0 0 0;
              0 0 0 0 0;
               0 0 0 0;
                0 0 0'
      assign_type = 'cell'
      id_name = assembly_id
  []

  [CORE_3D_BASE]
    type = AdvancedExtruderGenerator
    input = 'CORE_2D'
    heights = '10.0 40.0 10.0'
    num_layers = '1 4 1'
    direction = '0 0 1'
  []

  [CORE_3D]
    type = PlaneIDMeshGenerator
    input = 'CORE_3D_BASE'
    plane_coordinates = '0. 10. 50. 60.'
    num_ids_per_plane = '1 4 1'
    id_name = 'plane_id'
  []
[]
