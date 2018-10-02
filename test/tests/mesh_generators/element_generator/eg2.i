[MeshGenerators]
  [./eg]
    type = ElementGenerator
    nodal_positions = '0 0 0
                       1 0 0
                       1 1 0
                       0 1 0'

    element_connectivity = '0 1 2 3'
  []

  [./eg2]
    type = ElementGenerator
    input = eg
    nodal_positions = '0 0 0
                       -1 0 0
                       -1 -1 0
                       0 -1 0'

    element_connectivity = '0 1 2 3'
  []
[]

[Mesh]
  type = MeshGeneratorMesh
[]

[Outputs]
  exodus = true
[]