[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [main]
    initial_condition = 1938
  []
[]

[MultiApps/sub]
  type = TransientMultiApp
  input_files = sub.i
[]

[Transfers/from_sub]
  type = MultiAppCopyTransfer
  from_multi_app = sub
  source_variable = sub
  variable = main
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  execute_on = 'FINAL'
  exodus = true
[]
