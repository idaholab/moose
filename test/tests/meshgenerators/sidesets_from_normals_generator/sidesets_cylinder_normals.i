[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = cylinder.e
    parallel_type = replicated
  []

  [./sidesets]
    type = SideSetsFromNormalsGenerator
    input = fmg
    normals = '0  0  1
               0  1  0
               0  0 -1'
    fixed_normal = false
    new_boundary = 'top side bottom'
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
