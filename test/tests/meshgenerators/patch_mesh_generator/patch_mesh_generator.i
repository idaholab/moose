[MeshGenerators]
  [pmg]
    type = PatchMeshGenerator
    dim = 3
    elem_type = HEX8
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
