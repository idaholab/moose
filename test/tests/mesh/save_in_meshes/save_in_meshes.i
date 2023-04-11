[Mesh]
  [A]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
    xmin = 3
    xmax = 6
    ymin = -5
    ymax = 5
  []
  [B]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 3
    ny = 3
    xmin = -3
    xmax = 0
    ymin = -5
    ymax = 5
  []

  [A_and_B]
    type = MeshCollectionGenerator
    inputs = 'A B'
  []
[]

[UserObjects/test]
  type = TestSaveInMesh
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

