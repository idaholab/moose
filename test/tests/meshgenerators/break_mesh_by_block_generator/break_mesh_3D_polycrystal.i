[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = poly2.msh
    #parallel_type = replicated
  []

  [./breakmesh]
    type = BreakMeshByBlockGenerator
    input = fmg
    split_interface = true
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
