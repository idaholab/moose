[Mesh]
  [cc]
    type = ConcentricCircleMeshGenerator
    num_sectors = 6
    radii = '1 2'
    rings = '2 2'
    has_outer_square = false
    preserve_volumes = true
  []
  [bdg]
    type = BlockDeletionGenerator
    input = 'cc'
    block = '1'
    new_boundary = '100'
  []
  [boundary_layer]
    type = BoundaryLayerTriangleGenerator
    input = 'bdg'
    thickness = 0.1
    num_layers = 3
    keep_input = true
    boundary_names = '100'
    boundary_layer_direction = 'INWARD'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Postprocessors]
  [num_elems]
    type = NumElements
  []
  [area]
    type = VolumePostprocessor
  []
[]

[Outputs]
  file_base = 'boundary_layer_stitch_inward'
  csv = true
  execute_on = 'FINAL'
[]
