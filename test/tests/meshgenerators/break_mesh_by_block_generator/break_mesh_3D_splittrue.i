[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = coh3D_3Blocks.e
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
