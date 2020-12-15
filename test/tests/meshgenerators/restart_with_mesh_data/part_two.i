[Mesh]
  [file]
    type = FileMeshGenerator
    file = part_one_out_cp/0000_mesh.cpr
  []
  [get]
    type = TestMeshDataGetGenerator
    input = file
    get_mesh_data_value = 1980
    prefix = declare # object name from part 1
  []
[]

[Problem]
  #restart_file_base = part_one_out_cp
  solve = false
[]

[Executioner]
  type = Steady
[]

[Outputs]
[]
