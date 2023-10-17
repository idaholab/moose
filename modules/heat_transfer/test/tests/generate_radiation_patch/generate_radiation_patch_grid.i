[Mesh]
  type = MeshGeneratorMesh

  [cmg]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = 3
    nx = 25
    ymin = 0
    ymax = 2
    ny = 25
    zmin = 0
    zmax = 4
    nz = 25
  []

  [patch]
    type = PatchSidesetGenerator
    boundary = back
    n_patches = 19
    input = cmg
    partitioner = grid
  []
[]
