# A component using a mesh from a mesh generator, modified from the command line to trigger
# each error
[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
  []
[]

[ActionComponents]
  [comp]
    type = MeshGeneratorComponent
    mesh_generator = 'gmg'
    mesh_generator_type = 'final_generator'
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]
