[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = cylinder.e
    parallel_type = replicated
  []

  [./sidesets]
    type = SideSetsFromPointsGenerator
    input = fmg
    points = '0   0  0.5
              0.1 0  0
              0   0 -0.5'
    new_boundary = 'top side bottom'
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
