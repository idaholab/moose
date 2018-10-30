[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = quad_mesh.e
  []

  [./fmg2]
    type = FileMeshGenerator
    file = tri_mesh.e
  []

  [./pmg]
    type = PatternedMeshGenerator
    inputs = 'fmg fmg2'
    pattern = '0 0 0 0 0 0 0 0 0 0 0 0 0 0 ;
               0 1 1 0 0 0 0 0 0 0 0 1 1 0 ;
               0 1 1 1 0 0 0 0 0 0 1 1 1 0 ;
               0 1 0 1 1 0 0 0 0 1 1 0 1 0 ;
               0 1 0 0 1 1 0 0 1 1 0 0 1 0 ;
               0 1 0 0 0 1 1 1 1 0 0 0 1 0 ;
               0 1 0 0 0 0 1 1 0 0 0 0 1 0 ;
               0 1 0 0 0 0 0 0 0 0 0 0 1 0 ;
               0 1 0 0 0 0 0 0 0 0 0 0 1 0 ;
               0 1 0 0 0 0 0 0 0 0 0 0 1 0 ;
               0 1 0 0 0 0 0 0 0 0 0 0 1 0 ;
               0 0 0 0 0 0 0 0 0 0 0 0 0 0'
    bottom_boundary = 1
    right_boundary = 2
    top_boundary = 3
    left_boundary = 4
    y_width = 1.2
    x_width = 1.2
    parallel_type = replicated
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
