[Mesh]
  [file]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 3
  []
  [mirror]
    type = SymmetryTransformGenerator
    input = file
    mirror_point = "0 1 0"
    mirror_normal_vector = "0 1 0"
  []
  [stitch]
    type = StitchedMeshGenerator
    inputs = 'file mirror'
    stitch_boundaries_pairs = 'top top'
  []
[]
