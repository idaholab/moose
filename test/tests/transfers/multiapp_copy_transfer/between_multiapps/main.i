[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[MultiApps/sub1]
  type = TransientMultiApp
  input_files = sub1.i
[]

[MultiApps/sub2]
  type = TransientMultiApp
  input_files = sub2.i
[]

[Transfers/from_sub1_to_sub2]
  type = MultiAppCopyTransfer
  from_multi_app = sub1
  to_multi_app = sub2
  source_variable = x1
  variable = x2
[]

[Executioner]
  type = Transient
  num_steps = 1
[]
