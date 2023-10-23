[Mesh]
  type = MeshGeneratorMesh

  [cmg]
    type = GeneratedMeshGenerator
    dim = 3
    xmin = 0
    xmax = 1
    nx = 25
    ymin = 0
    ymax = 1
    ny = 25
    zmin = 0
    zmax = 1
    nz = 25
  []

  [patch]
    type = PatchSidesetGenerator
    boundary = 0
    n_patches = 10
    input = cmg
  []
[]
