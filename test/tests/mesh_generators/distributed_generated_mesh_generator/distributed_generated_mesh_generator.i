[MeshGenerators]
  [dgmg]
    type = DistributedGeneratedMeshGenerator
    nx = 10
    ny = 10
    nz = 10
    dim = 3
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
