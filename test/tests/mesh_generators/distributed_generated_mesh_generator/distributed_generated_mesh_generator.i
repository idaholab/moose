[MeshGenerators]
  [dgmg]
    type = DistributedGeneratedMeshGenerator
    dim = 3
    nx = 3
    nz = 4
    bias_x = 2
    bias_z = 0.5
    ny = 3
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
