# This mesh uses XYZDelaunayGenerator to mesh a cavity between a shield and
# reactor core. A stitch occurs, where the nodes match, but the faces do not,
# resulting in a stitch failure.

[Mesh]
  [cartesian_mg]
    type = CartesianMeshGenerator
    dim = 3
    dx = '1 5 1'
    ix = '2 5 2'
    dy = '1 5 1'
    iy = '2 5 2'
    dz = '1 5 1'
    iz = '2 5 2'
  []
  [renumber_cartesian_mg]
    type = RenameBlockGenerator
    input = cartesian_mg
    old_block = 0
    new_block = 100
  []
  [shield_mg]
    type = RenameBlockGenerator
    input = renumber_cartesian_mg
    old_block = 100
    new_block = shield
  []
  [cavity_mg]
    type = ParsedSubdomainMeshGenerator
    input = shield_mg
    combinatorial_geometry = 'x > 1 & x < 6 & y > 1 & y < 6 & z > 1 & z < 6'
    block_id = 101
    block_name = 'cavity'
  []
  [remove_cavity_mg]
    type = BlockDeletionGenerator
    input = cavity_mg
    block = cavity
    new_boundary = shield_inner
  []

  [core_mg]
    type = CartesianMeshGenerator
    dim = 3
    dx = '2'
    ix = '4'
    dy = '2'
    iy = '4'
    dz = '2'
    iz = '4'
  []
  [translate_core_mg]
    type = TransformGenerator
    input = core_mg
    transform = TRANSLATE
    vector_value = '2.5 2.5 2.5'
  []
  [renumber_core_mg]
    type = RenameBlockGenerator
    input = translate_core_mg
    old_block = 0
    new_block = 200
  []
  [name_core_mg]
    type = RenameBlockGenerator
    input = renumber_core_mg
    old_block = 200
    new_block = core
  []

  # generate cavity mesh; 3 steps:
  #   1. create lower-D block of the shield inner surface
  #   2. extract that into its own mesh
  #   3. provide that and the core mesh to XYZDelaunayGenerator to generate cavity mesh
  [shield_inner_block_mg]
    type = LowerDBlockFromSidesetGenerator
    input = remove_cavity_mg
    sidesets = 'shield_inner'
    new_block_id = 300
    new_block_name = shield_inner_block
  []
  [extract_shield_inner_block_mg]
    type = BlockToMeshConverterGenerator
    input = shield_inner_block_mg
    target_blocks = 'shield_inner_block'
  []
  [cavity_xyz_mg]
    type = XYZDelaunayGenerator
    boundary = 'extract_shield_inner_block_mg'
    holes = 'name_core_mg'
    output_subdomain_name = 'cavity'
    output_boundary = 'cavity_outer'
    desired_volume = 1.0
    stitch_holes = true
    convert_holes_for_stitching = true
    conversion_method = SURFACE
  []
  [rename_core_surface_blocks_mg]
    type = RenameBlockGenerator
    input = cavity_xyz_mg
    old_block = '401 602'
    new_block = 'core_tet core_pyr'
  []
  [stitch_mg]
    type = StitchMeshGenerator
    inputs = 'remove_cavity_mg rename_core_surface_blocks_mg'
    stitch_boundaries_pairs = 'shield_inner cavity_outer'
    enforce_all_nodes_match_on_boundaries = true
  []
  [core_outer_mg]
    type = SideSetsBetweenSubdomainsGenerator
    input = stitch_mg
    primary_block = core_tet
    paired_block = cavity
    new_boundary = core_outer
  []
[]
