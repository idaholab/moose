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

  [C]
    type = GeneratedMeshGenerator
    dim = 1
    nx = 3
    xmin = -3
    xmax = 0
    save_with_name = 'C'
  []

  [A_and_B]
    type = MeshCollectionGenerator
    inputs = 'A B'
  []

  final_generator = 'A_and_B'
[]

[Outputs]
  exodus = true
[]
