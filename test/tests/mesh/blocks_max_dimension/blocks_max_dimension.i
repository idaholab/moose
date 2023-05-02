# This input file tests MooseMesh::getBlocksMaxDimension(), which gets the MESH
# dimension of a list of subdomain names.
#
# Note the differences between the MESH dimension and the SPATIAL dimension.
# The SPATIAL dimension just looks at the maximum coordinate dimension used:
#   - Equals 3 if there is a nonzero z coordinate
#   - Equals 2 if there is no nonzero z coordinate, but there is a nonzero y coordinate
#   - Equals 1 if there is no nonzero y or z coordinate
# In contrast, the MESH dimension looks at the dimensionality of the elements.
# Therefore, the MESH dimension differs from the SPATIAL dimension when:
#   - a 1D element has a nonzero y or z coordinate
#   - a 2D element has a nonzero z coordinate
# This test will include subdomains with these cases and test different
# lists of subdomains.
#

[Mesh]
  # 1D block
  [block1d_mg]
    type = GeneratedMeshGenerator
    dim = 1
    xmin = 0.0
    xmax = 1.0
  []
  [block1d_renumber_mg]
    type = RenameBlockGenerator
    input = block1d_mg
    old_block = 0
    new_block = 1
  []
  [block1d_rename_mg]
    type = RenameBlockGenerator
    input = block1d_renumber_mg
    old_block = 1
    new_block = 'block1d'
  []
  [block1d_translate_mg]
    type = TransformGenerator
    input = block1d_rename_mg
    transform = TRANSLATE
    vector_value = '0 0 1.0'
  []

  # 2D block
  [block2d_mg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 2.0
    xmax = 3.0
    ymin = 0.0
    ymax = 1.0
    boundary_id_offset = 10
  []
  [block2d_renumber_mg]
    type = RenameBlockGenerator
    input = block2d_mg
    old_block = 0
    new_block = 2
  []
  [block2d_rename_mg]
    type = RenameBlockGenerator
    input = block2d_renumber_mg
    old_block = 2
    new_block = 'block2d'
  []
  [boundary2d_rename_mg]
    type = RenameBoundaryGenerator
    input = block2d_rename_mg
    old_boundary = 'left right bottom top'
    new_boundary = 'left2d right2d bottom2d top2d'
  []
  [block2d_translate_mg]
    type = TransformGenerator
    input = boundary2d_rename_mg
    transform = TRANSLATE
    vector_value = '0 0 1.0'
  []

  # 3D block
  [block3d_mg]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 4.0
    xmax = 5.0
    ymin = 0.0
    ymax = 1.0
    zmin = 0.0
    zmax = 1.0
    boundary_id_offset = 20
  []
  [block3d_renumber_mg]
    type = RenameBlockGenerator
    input = block3d_mg
    old_block = 0
    new_block = 3
  []
  [block3d_rename_mg]
    type = RenameBlockGenerator
    input = block3d_renumber_mg
    old_block = 3
    new_block = 'block3d'
  []
  [boundary3d_rename_mg]
    type = RenameBoundaryGenerator
    input = block3d_rename_mg
    old_boundary = 'left right bottom top back front'
    new_boundary = 'left3d right3d bottom3d top3d back3d front3d'
  []

  # combine blocks
  [combiner_mg]
    type = CombinerGenerator
    inputs = 'block1d_translate_mg block2d_translate_mg boundary3d_rename_mg'
  []
[]

[Postprocessors]
  [dim_1d]
    type = BlocksMaxDimensionPostprocessor
    block = 'block1d'
    execute_on = 'INITIAL'
  []
  [dim_1d_2d]
    type = BlocksMaxDimensionPostprocessor
    block = 'block1d block2d'
    execute_on = 'INITIAL'
  []
  [dim_1d_2d_3d]
    type = BlocksMaxDimensionPostprocessor
    block = 'block1d block2d block3d'
    execute_on = 'INITIAL'
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Outputs]
  csv = true
  execute_on = 'INITIAL'
[]
