[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
[]

[MultiApps/sub]
  type = TransientMultiApp
  input_files = sub.i
[]

[Transfers/from_sub]
  type = MultiAppCopyTransfer
  to_multi_app = sub
  source_variable = x
  variable = aux
[]

[AuxVariables/x]
  initial_condition = 1949
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[Outputs]
  execute_on = 'FINAL'
  exodus = true
[]
