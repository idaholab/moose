[Mesh/file]
  type = FileMeshGenerator
  file = receiver_initial_out_cp/0001-mesh.cpa.gz
[]

[Postprocessors/constant]
  type = Receiver
[]

[Problem]
  solve = false
  restart_file_base = receiver_initial_out_cp/0001
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
