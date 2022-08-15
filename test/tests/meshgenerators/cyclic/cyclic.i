[Mesh]
  [ccmg]
    type = ConcentricCircleMeshGenerator
    num_sectors = 6
    radii = '0.2546 0.3368'
    rings = '4 3 4'
    has_outer_square = on
    pitch = 1
    preserve_volumes = off
    smoothing_max_it = 3
  []
  [rename_left]
    type = RenameBoundaryGenerator
    input = ccmg
    old_boundary = 'left'
    new_boundary = '101'
  []
  [left]
    type = CartesianMeshGenerator
    dim = 2
    dx = '5'
    dy = '1'
    ix = '100'
    iy = '16'
  []
  [move_it]
    type = TransformGenerator
    input = left
    transform = translate
    vector_value = '-5.5 -0.5 0'
  []
  [rename_middle]
    type = RenameBoundaryGenerator
    input = move_it
    old_boundary = 'right'
    new_boundary = '102'
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'rename_left rename_middle'
    stitch_boundaries_pairs = '101 102'
  []
  [in_between]
    type = SideSetsBetweenSubdomainsGenerator
    input = stitch
    primary_block = 2
    paired_block = 1
    new_boundary = 'no_circle'
  []
  [delete]
    type = BlockDeletionGenerator
    input = in_between
    block = '1'
  []
  [create_fused_top_sideset_l]
    input = delete
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y > 0.49'
    normal = '0 1 0'
    new_sideset_name = 103
  []
  [top_left_block]
    type = GeneratedMeshGenerator
    xmin = -5.5
    xmax = -0.5
    ymin = 0.5
    ymax = ${fparse 0.5 + 2. / 16.}
    nx = 100
    ny = 2
    dim = 2
  []
  [rename_top_left_block]
    input = top_left_block
    type = RenameBlockGenerator
    old_block = '0'
    new_block = '100'
  []
  [rename_middle_2]
    input = rename_top_left_block
    type = RenameBoundaryGenerator
    old_boundary = 'right'
    new_boundary = '104'
  []
  [top_middle_block]
    type = GeneratedMeshGenerator
    xmin = -0.5
    xmax = 0.5
    ymin = 0.5
    ymax = ${fparse 0.5 + 2. / 16.}
    nx = 16
    ny = 2
    dim = 2
  []
  [rename_top_middle_block]
    input = top_middle_block
    type = RenameBlockGenerator
    old_block = '0'
    new_block = '101'
  []
  [rename_left_2]
    input = rename_top_middle_block
    type = RenameBoundaryGenerator
    old_boundary = 'left'
    new_boundary = '105'
  []
  [stitch_2]
    inputs = 'rename_middle_2 rename_left_2'
    type = StitchedMeshGenerator
    stitch_boundaries_pairs = '104 105'
  []
  [create_fused_bottom_sideset]
    input = stitch_2
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y < 0.51'
    normal = '0 -1 0'
    new_sideset_name = 106
  []
  [stitch_3]
    inputs = 'create_fused_top_sideset_l create_fused_bottom_sideset'
    type = StitchedMeshGenerator
    stitch_boundaries_pairs = '103 106'
  []
  [rename_extension]
    type = RenameBoundaryGenerator
    input = no_slip_bottom
    old_boundary = 'extension'
    new_boundary = '111'
  []
  [extension]
    type = CartesianMeshGenerator
    dim = 2
    dx = '5'
    dy = '1'
    ix = '100'
    iy = '16'
  []
  [move_it_2]
    type = TransformGenerator
    input = extension
    transform = translate
    vector_value = '5.5 -0.5 0'
  []
  [stitch_4]
    inputs = 'rename_middle rename_extension'
    type = StitchedMeshGenerator
    stitch_boundaries_pairs = '102 111'
  []
  [create_fused_top_sideset_r]
    input = stitch_4
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y > .49'
    normal = '0 1 0'
    new_sideset_name = 113
  []
  [top_right_block]
    type = GeneratedMeshGenerator
    xmin = 0.5
    xmax = 5.5
    ymin = 0.5
    ymax = ${fparse 0.5 + 2. / 16.}
    nx = 100
    ny = 2
    dim = 2
  []
  [rename_top_right_block]
    input = top_right_block
    type = RenameBlockGenerator
    old_block = 'top_right_block'
    new_block = '110'
  []
  [rename_extension_2]
    input = rename_top_right_block
    type = RenameBoundaryGenerator
    old_boundary = 'rename_extension'
    new_boundary = '115'
  []
  [stitch_5]
    inputs = 'rename_middle_2 rename_extension_2'
    type = StitchedMeshGenerator
    stitch_boundaries_pairs = '104 115'
  []
  [stitch_6]
    inputs = 'create_fused_bottom_sideset create_fused_top_sideset_r'
    type = StitchedMeshGenerator
    stitch_boundaries_pairs = '106 113'
  []
  [no_slip_top]
    input = stitch_6
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y > .615'
    normal = '0 1 0'
    new_sideset_name = 'no_slip_top'
  []
  [no_slip_bottom]
    input = no_slip_top
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y < -0.49'
    normal = '0 -1 0'
    new_sideset_name = 'no_slip_bottom'
  []
  [inlet]
    input = no_slip_bottom
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x > 5.49'
    normal = '1 0 0'
    new_sideset_name = 'inlet'
  []
  [outlet]
    input = inlet
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x < -5.49'
    normal = '-1 0 0'
    new_sideset_name = 'outlet'
  []
[]
