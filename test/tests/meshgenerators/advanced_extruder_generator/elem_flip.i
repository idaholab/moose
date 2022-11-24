[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    elem_type = QUAD4
  []
  [mirror]
    type = TransformGenerator
    input = gmg
    transform = ROTATE
    vector_value = '0 180 0'
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'gmg mirror'
    stitch_boundaries_pairs = 'bottom bottom'
  []
  [feg]
    type = AdvancedExtruderGenerator
    input = stitch
    direction = '0 0 1'
    num_layers = 1
    heights = 1
    top_boundary = 100
    bottom_boundary = 200
  []
[]
