[MeshGenerators]
  [./fmg]
    type = FileMeshGenerator
    file = cylinder.e
  []

  [./rotate]
    type = TransformGenerator
    input = fmg
    transform = ROTATE
    vector_value = '0 90 0'
  []

  [./scale]
    type = TransformGenerator
    input = rotate
    transform = SCALE
    vector_value ='1e2 1e2 1e2'
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]
