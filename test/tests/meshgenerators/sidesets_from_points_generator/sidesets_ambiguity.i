[GlobalParams]
  prevent_boundary_ids_overlap = false
[]

[Mesh]
  [region_2_gen]
      type = CartesianMeshGenerator
      dim = 2
      dx = '0.065 0.13 0.305 0.17 0.196'
      ix = '  2    2     2    2     2'
      dy = '0.85438 '
      iy = '6'
      subdomain_id = '68 68 68 68 68'
  []
  [region_2_move]
      type = TransformGenerator
      transform = TRANSLATE
      vector_value = '1.2 1.551 0'
      input = region_2_gen
  []
  [region_3_gen]
      type = CartesianMeshGenerator
      dim = 2
      dx = '0.24 0.24 0.24 0.24 0.24'
      ix = ' 2   2   2   2   2'
      dy = '0.744166666666666 0.744166666666667 0.744166666666667'
      iy = ' 2 2 2'
      subdomain_id = '56 57 58 59 60
                      51 52 53 54 55
                      46 47 48 49 50'
  []
  [region_3_move]
      type = TransformGenerator
      transform = TRANSLATE
      vector_value = '0 2.40538 0'
      input = region_3_gen
  []
  [region_1_gen]
      type = GeneratedMeshGenerator
      dim = 2
      nx = 10
      ny = 6
      xmin = 0
      xmax = 0.26
      ymin = 1.551
      ymax = 1.851
      subdomain_ids = '62 62 62 62 62 62 62 62 62 62
                       62 62 62 62 62 62 62 62 62 62
                       62 62 62 62 62 62 62 62 62 62
                       62 62 62 62 62 62 62 62 62 62
                       62 62 62 62 62 62 62 62 62 62
                       62 62 62 62 62 62 62 62 62 62'
  []
  [region_1_extend_1]
      type = FillBetweenSidesetsGenerator
      input_mesh_1 = 'region_3_move'
      input_mesh_2 = 'region_1_gen'
      boundary_1 = '0'
      boundary_2 = '2'
      num_layers = 6
      block_id= 61
      use_quad_elements = true
      keep_inputs = true
      begin_side_boundary_id = '3'
      end_side_boundary_id = '1'
  []
  [region_1_extend_2]
      type = FillBetweenSidesetsGenerator
      input_mesh_1 = 'region_2_move'
      input_mesh_2 = 'region_1_gen'
      boundary_1 = 3
      boundary_2 = 1
      num_layers = 6
      block_id= 69
      use_quad_elements = true
      keep_inputs = false
      begin_side_boundary_id = '0'
      end_side_boundary_id = '3'
      input_boundary_1_id = '1'
      input_boundary_2_id = '3'
  []
  [region_2_2_gen]
      type = CartesianMeshGenerator
      dim = 2
      dx = '0.065 0.13 0.305 0.17 0.196'
      ix = '  2    2     2    2     2'
      dy = '0.85438 '
      iy = '6'
      subdomain_id = '68 68 68 68 68'
  []
  [region_2_2_move]
      type = TransformGenerator
      transform = TRANSLATE
      vector_value = '1.2 1.551 0'
      input = region_2_2_gen
  []
  [region_6_gen]
      type = CartesianMeshGenerator
      dim = 2
      dx = '0.26 0.94 0.065 0.13 0.305 0.17 0.196'
      ix = '10  6     2    2     2    2     2'
      dy = '0.584 0.967'
      iy = '  4    6'
      subdomain_id = '62 72 72 72 72 72 72
                      62 70 71 71 71 71 71'
  []
  [stitch_1_2_6]
      type = StitchedMeshGenerator
      inputs = 'region_1_extend_1 region_1_extend_2 region_2_2_move region_6_gen'
      stitch_boundaries_pairs = '1   3;
                                 1   3;
                                 0   0' # 0 0 will leave a split mesh
      merge_boundaries_with_same_name = false
  []
  [rename_boundary_stitch_1_2_6]
      type = RenameBoundaryGenerator
      input = stitch_1_2_6
      old_boundary = '1'
      new_boundary = '2'
  []
  [region_4_gen]
      type = CartesianMeshGenerator
      dim = 2
      dx = '0.065 0.13'
      ix = '  2    2  '
      dy = '0.744166666666666 0.744166666666667 0.744166666666667'
      iy = ' 2 2 2'
      subdomain_id = '78 92
                      78 91
                      78 90'
  []
  [region_4_move]
      type = TransformGenerator
      transform = TRANSLATE
      vector_value = '1.2 2.40538 0'
      input = region_4_gen
  []
  [region_5_gen]
      type = CartesianMeshGenerator
      dim = 2
      dx = '0.17 0.196'
      ix = '2     2'
      dy = '0.39  1.8425'
      iy = '2 4'
      subdomain_id = '100 104
                      100 104'
  []
  [region_5_move]
      type = TransformGenerator
      transform = TRANSLATE
      vector_value = '1.7 2.40538 0'
      input = region_5_gen
  []
  [region_5_extend]
      type = FillBetweenSidesetsGenerator
      input_mesh_1 = 'region_4_move'
      input_mesh_2 = 'region_5_move'
      boundary_1 = 1
      boundary_2 = 3
      num_layers = 2
      block_id= 96
      use_quad_elements = true
      keep_inputs = true
      begin_side_boundary_id = '0'
      end_side_boundary_id = '2'
  []
  [rename_boundary_region_5]
      type = RenameBoundaryGenerator
      input = region_5_extend
      old_boundary = '0'
      new_boundary = '3'
  []
  [stitch_1_2_6_5]
      type = StitchedMeshGenerator
      inputs = 'rename_boundary_stitch_1_2_6 rename_boundary_region_5'
      stitch_boundaries_pairs = '2     3;'
      merge_boundaries_with_same_name = false
  []
  [region_7_gen]
      type = CartesianMeshGenerator
      dim = 2
      dx = '0.24 0.24 0.24 0.24 0.24 0.065 0.13 0.305 0.17 0.196'
      ix = '  2    2    2    2    2      2    2     2    2     2'
      dy = '0.744166666666667 0.744166666666667 0.744166666666667 0.744166666666667
            0.744166666666667 0.744166666666667 0.744166666666666 0.744166666666666
            0.744166666666666 0.458 0.86002'
      iy = '2 2 2 2 2 2 2 2 2 2 4'
      subdomain_id = '41 42 43 44 45 77 89 95 99 103
                      36 37 38 39 40 77 88 95 99 103
                      31 32 33 34 35 77 87 95 99 103
                      26 27 28 29 30 76 86 94 98 102
                      21 22 23 24 25 76 85 94 98 102
                      16 17 18 19 20 76 84 94 98 102
                      11 12 13 14 15 75 83 93 97 101
                       6  7  8  9 10 75 82 93 97 101
                       1  2  3  4  5 75 81 93 97 101
                      67 67 67 67 67 74 80 65 65  66
                      63 63 63 63 63 73 79 64 64  64'
  []
  [region_7_move]
      type = TransformGenerator
      transform = TRANSLATE
      vector_value = '0.0 4.63788 0'
      input = region_7_gen
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'stitch_1_2_6_5 region_7_move'
    stitch_boundaries_pairs = '2 0'
    merge_boundaries_with_same_name = false
  []
  [rename_boundary_1]
    type = BoundaryDeletionGenerator
    input = stitch
    boundary_names = '0 1 2 3'
  []
  [rename_boundary_2]
      type = SideSetsFromPointsGenerator
      input = rename_boundary_1
      new_boundary = '4'
      # the point here is on the slit; ambiguous.
      points = '2.066 1.551 0.'
  []
[]
