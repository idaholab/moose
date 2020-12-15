[Mesh]
  [generate]
    type = GeneratedMeshGenerator
    dim = 1
  []
  [declare]
    type = TestMeshDataDeclareGenerator
    input = generate
    declare_mesh_data_value = 1980
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
  [out]
    type = Checkpoint
    execute_on = INITIAL
  []
[]
