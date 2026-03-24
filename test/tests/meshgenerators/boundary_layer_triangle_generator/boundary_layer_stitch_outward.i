[Mesh]
  [cc]
    type = ConcentricCircleMeshGenerator
    num_sectors = 6
    radii = '1'
    rings = '2'
    has_outer_square = false
    portion = left_half
    preserve_volumes = true
  []
  [boundary_layer]
    type = BoundaryLayerTriangleGenerator
    input = 'cc'
    thickness = 0.1
    num_layers = 3
    keep_input = true
  []
[]

[Problem]
  solve = false
[]

[AuxVariables]
  [unity]
    family = MONOMIAL
    order = CONSTANT
    initial_condition = 1.0
  []
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
  file_base = 'boundary_layer_stitch_outward'
  csv = true
  execute_on = 'FINAL'
[]
