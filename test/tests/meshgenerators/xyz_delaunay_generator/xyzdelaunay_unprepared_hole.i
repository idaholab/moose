# A hollow hex shell, straight out of an extruder, serves as an unstitched hole mesh.
# Its surface sides are QUAD4, so the generator converts the hole with all_tri(), which
# requires a prepared mesh.
[Mesh]
  [outer]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 1
    ny = 1
    nz = 1
    xmin = -4
    xmax = 4
    ymin = -4
    ymax = 4
    zmin = -4
    zmax = 4
  []
  [inner]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
  []
  [surf]
    type = LowerDBlockFromSidesetGenerator
    input = 'inner'
    sidesets = 'left right top bottom front back'
    new_block_name = 'surf'
  []
  [surf_mesh]
    type = BlockToMeshConverterGenerator
    input = 'surf'
    target_blocks = 'surf'
  []
  [shell]
    type = AdvancedExtruderGenerator
    input = 'surf_mesh'
    num_layers = '1'
    heights = '0.5'
    extrude_along_node_normals = true
    bottom_boundary = 'base'
    top_boundary = 'tip'
  []
  [triang]
    type = XYZDelaunayGenerator
    boundary = 'outer'
    holes = 'shell'
    hole_boundaries = 'tip'
    desired_volume = 4
  []
[]

[Executioner]
  type = Steady
[]

[Problem]
  solve = false
[]

[Postprocessors]
  [volume]
    type = VolumePostprocessor
  []
[]

[Outputs]
  csv = true
[]
