[Mesh]
  [left]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1'
    dy = '1'
    ix = '12'
    iy = '12'
  []
  [translation]
    type = TransformGenerator
    input = 'left'
    transform = 'TRANSLATE'
    vector_value = '-0.5 0.5 0'
  []
  [ccmg]
    type = ConcentricCircleMeshGenerator
    num_sectors = 6
    radii = '0.25'
    rings = '2 2'
    has_outer_square = on
    pitch = 1
    smoothing_max_it = 3
    preserve_volumes = true
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'translation ccmg'
    stitch_boundaries_pairs = 'right left'
  []
[]
