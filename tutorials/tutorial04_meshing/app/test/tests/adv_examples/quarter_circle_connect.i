l_lat  = 5  # length of a lattice edge
l_fuel = 3 # length of the fuel channel (including round end)
r_fuel = 0.5 # radius of the round end

[Mesh]
   [Corner_bottom_left_1]
    type = ConcentricCircleMeshGenerator
    num_sectors = 8
    radii = '${fparse r_fuel} ${fparse r_fuel}'
    rings = '1 1'
    has_outer_square = off
    pitch = 2
    portion = top_right
    preserve_volumes = off
   []
   [Corner_bottom_left_2]
     type = BlockDeletionGenerator
     input = Corner_bottom_left_1
     block = '2'
     new_boundary = 'curve_1'
   []
    [Corner_top_right_1]
     type = ConcentricCircleMeshGenerator
     num_sectors = 8
     radii = ${fparse r_fuel}
     rings = '1 1'
     has_outer_square = on
     pitch = 2
     portion = bottom_left
     preserve_volumes = off
   []
   [Corner_top_right_2]
     type = BlockDeletionGenerator
     input = Corner_top_right_1
     block = '2'
     new_boundary = 'curve_2'
   []
   [Corner_top_right_3]
     type = TransformGenerator
     transform = TRANSLATE
     vector_value = '${fparse (l_lat-l_fuel)/2+r_fuel} ${fparse (l_lat-l_fuel)/2+r_fuel}  0'
     input = Corner_top_right_2
   []
   [connect_two_circles]
     type = FillBetweenSidesetsGenerator
     input_mesh_1 = 'Corner_bottom_left_2'
     input_mesh_2 = 'Corner_top_right_3'
     boundary_1 = 'curve_1'
     boundary_2 = 'curve_2'
     num_layers = 8
     keep_inputs = true
     use_quad_elements = true
     block_id = 200
   []
[]
