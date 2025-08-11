[Mesh]
  # ------------------------------------------
  # Middle layer
  # ------------------------------------------
  [ccmg]
    type = ConcentricCircleMeshGenerator
    num_sectors = '${sectors}'
    radii = '${circle_radius} ${fparse 1.2*circle_radius} ${fparse 1.4*circle_radius}'
    rings = '4 10 5 10'
    has_outer_square = on
    pitch = ${pitch}
    preserve_volumes = off
    smoothing_max_it = 2
  []
  [in_between]
    type = SideSetsBetweenSubdomainsGenerator
    input = ccmg
    primary_block = 2
    paired_block = 1
    new_boundary = 'circle'
  []
  [delete]
    type = BlockDeletionGenerator
    input = in_between
    block = '1'
  []
  [final_ccmg]
    type = RenameBlockGenerator
    input = delete
    old_block = '2 3'
    new_block = '0 0'
  []
  [left]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = '${x_min}'
    xmax = '${fparse -pitch/2}'
    ymin = '${fparse -pitch/2}'
    ymax = '${fparse pitch/2}'
    nx = '24'
    ny = '54'
  []
  [right]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = '${fparse pitch/2}'
    xmax = '${x_max}'
    ymin = '${fparse -pitch/2}'
    ymax = '${fparse pitch/2}'
    nx = '120'
    ny = '54'
    bias_x = 1.015
  []
  [combined_middle]
    type = StitchedMeshGenerator
    inputs = 'final_ccmg left right'
    stitch_boundaries_pairs = 'left right; right left'
    clear_stitched_boundary_ids = false
    prevent_boundary_ids_overlap = true
    merge_boundaries_with_same_name = true
  []

  [middle_top_sideset]
    input = combined_middle
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y > ${fparse pitch/2-rundoff}'
    normal = '0 1 0'
    new_sideset_name = 'middle_top'
  []
  [middle_bottom_sideset]
    input = middle_top_sideset
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y < ${fparse -pitch/2+rundoff}'
    normal = '0 -1 0'
    new_sideset_name = 'middle_bottom'
  []
  # ------------------------------------------
  # Top layer
  # ------------------------------------------
  [top_left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = '${x_min}'
    xmax = '${fparse -pitch/2}'
    ymin = '${fparse pitch/2}'
    ymax = '${y_max}'
    nx = '24'
    ny = '24'
  []
  [top_middle_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = '${fparse -pitch/2}'
    xmax = '${fparse pitch/2}'
    ymin = '${fparse pitch/2}'
    ymax = '${y_max}'
    nx = '54'
    ny = '24'
  []
  [top_right_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = '${fparse pitch/2}'
    xmax = '${x_max}'
    ymin = '${fparse pitch/2}'
    ymax = '${y_max}'
    nx = '120'
    ny = '24'
    bias_x = 1.015
  []
  [combined_top]
    type = StitchedMeshGenerator
    inputs = 'top_middle_block top_left_block top_right_block'
    stitch_boundaries_pairs = 'left right; right left'
    prevent_boundary_ids_overlap = true
    merge_boundaries_with_same_name = true
  []
  [top_bottom_sideset]
    input = combined_top
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y < ${fparse pitch/2+rundoff}'
    normal = '0 -1 0'
    new_sideset_name = 'top_bottom'
  []
  [combined_middle_top]
    type = StitchedMeshGenerator
    inputs = 'top_bottom_sideset middle_bottom_sideset'
    stitch_boundaries_pairs = 'top_bottom middle_top'
    clear_stitched_boundary_ids = false
    prevent_boundary_ids_overlap = true
    merge_boundaries_with_same_name = true
  []
  [create_fused_top_sideset]
    input = combined_middle_top
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y > ${fparse y_max-rundoff}'
    normal = '0 1 0'
    new_sideset_name = 'top_boundary'
  []
  # ------------------------------------------
  # Bottom layer
  # ------------------------------------------
  [bottom_left_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = '${x_min}'
    xmax = '${fparse -pitch/2}'
    ymin = '${y_min}'
    ymax = '${fparse -pitch/2}'
    nx = '24'
    ny = '24'
  []
  [bottom_middle_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = '${fparse -pitch/2}'
    xmax = '${fparse pitch/2}'
    ymin = '${y_min}'
    ymax = '${fparse -pitch/2}'
    nx = '54'
    ny = '24'
  []
  [bottom_right_block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = '${fparse pitch/2}'
    xmax = '${x_max}'
    ymin = '${y_min}'
    ymax = '${fparse -pitch/2}'
    nx = '120'
    ny = '24'
    bias_x = 1.015
  []
  [combined_bottom]
    type = StitchedMeshGenerator
    inputs = 'bottom_middle_block bottom_left_block bottom_right_block'
    stitch_boundaries_pairs = 'left right; right left'
    prevent_boundary_ids_overlap = true
    merge_boundaries_with_same_name = true
  []
  [bottom_top_sideset]
    input = combined_bottom
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y > ${fparse -pitch/2-rundoff}'
    normal = '0 1 0'
    new_sideset_name = 'bottom_top'
  []
  [combined_final]
    type = StitchedMeshGenerator
    inputs = 'create_fused_top_sideset bottom_top_sideset'
    stitch_boundaries_pairs = 'middle_bottom bottom_top'
    clear_stitched_boundary_ids = false
    prevent_boundary_ids_overlap = true
    merge_boundaries_with_same_name = true
  []
  [create_fused_bottom_sideset]
    input = combined_final
    type = ParsedGenerateSideset
    combinatorial_geometry = 'y < ${fparse y_min+rundoff}'
    normal = '0 -1 0'
    new_sideset_name = 'bottom_boundary'
  []
  # ------------------------------------------
  # Left and right boundaries
  # ------------------------------------------
  [create_fused_left_sideset]
    input = create_fused_bottom_sideset
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x < ${fparse x_min+rundoff}'
    normal = '-1 0 0'
    new_sideset_name = 'left_boundary'
  []
  [create_fused_right_sideset]
    input = create_fused_left_sideset
    type = ParsedGenerateSideset
    combinatorial_geometry = 'x > ${fparse x_max-rundoff}'
    normal = '1 0 0'
    new_sideset_name = 'right_boundary'
  []
  [sideset_removal]
    input = create_fused_right_sideset
    type = BoundaryDeletionGenerator
    boundary_names = 'bottom top left right middle_bottom middle_top bottom_top top_bottom'
  []
  [fluid_block]
    type = RenameBlockGenerator
    input = sideset_removal
    old_block = '0     4    '
    new_block = 'fluid fluid'
  []
  [ccmg_solid]
    type = ConcentricCircleMeshGenerator
    num_sectors = '${sectors}'
    radii = '${fparse 0.5*dia}'
    rings = '5'
    has_outer_square = false
    pitch = 1.42063
    preserve_volumes = off
    smoothing_max_it = 3
  []
  [solid_block]
    type = RenameBlockGenerator
    input = ccmg_solid
    old_block = '1'
    new_block = 'solid'
  []
  [smg]
    type = StitchedMeshGenerator
    inputs = 'fluid_block solid_block'
    stitch_boundaries_pairs = 'circle outer'
    show_info = true
    prevent_boundary_ids_overlap = false
    clear_stitched_boundary_ids = true
  []
  [interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = 'smg'
    primary_block = 'fluid'
    paired_block  = 'solid'
    new_boundary ='circle'
  []
#  [refine]
#    type = RefineBlockGenerator
#    input = interface
#    block = 'fluid solid'
#    refinement = '1 1'
#  []
[]
