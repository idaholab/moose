[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []

  [rotate]
    type = TransformGenerator
    input = gmg
    transform = ROTATE
    vector_value = '0 90 0'
  []
[]

[Variables]
  [u]
  []
[]

# Since this mesh is rotated into the z-plane, we need to output in 3D
# This should occur automatically
