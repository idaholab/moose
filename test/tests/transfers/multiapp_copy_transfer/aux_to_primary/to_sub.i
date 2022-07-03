[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[Variables]
  [main]
    initial_condition = 1949
  []
[]

[MultiApps/sub]
  type = TransientMultiApp
  input_files = sub.i
[]

[Transfers/to_sub]
  type = MultiAppCopyTransfer
  to_multi_app = sub
  source_variable = main
  variable = sub
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  execute_on = 'FINAL'
  exodus = true
[]
