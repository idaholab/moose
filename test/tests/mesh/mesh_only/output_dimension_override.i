[MeshGenerators]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
[]

[MeshModifiers]
  [rotate]
    type = Transform
    transform = ROTATE
    vector_value = '0 90 0'
  []
[]

[Variables]
  [u]
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  [out]
    type = Exodus
    # Since this mesh is rotated into the z-plane, we need to output in 3D
    output_dimension = 3
  []
[]
