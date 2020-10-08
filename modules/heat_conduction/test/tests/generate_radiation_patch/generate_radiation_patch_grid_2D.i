[Mesh]
  [cartesian]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1   2  3'
    ix = '20 20 20'
    dy = '5'
    iy = '10'
    subdomain_id = '1 2 3'
  []

  [rotate]
    type = TransformGenerator
    transform = ROTATE
    vector_value = '0 0 -45'
    input = cartesian
  []

  [patch]
    type = PatchSidesetGenerator
    boundary = 0
    n_patches = 4
    input = rotate
    partitioner = grid
  []
[]
