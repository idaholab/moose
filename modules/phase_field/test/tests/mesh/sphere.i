[MeshGenerators]
  [./sphere]
    type = SphereSurfaceMeshGenerator
    center = '1 2 3'
    radius = 4
    depth = 2
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
