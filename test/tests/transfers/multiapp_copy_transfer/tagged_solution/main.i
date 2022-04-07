[Problem]
  solve = false
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  ny = 4
[]

[MultiApps/sub]
  type = FullSolveMultiApp
  input_files = sub.i
[]

[Transfers/to_sub]
  type = MultiAppCopyTransfer
  to_multi_app = sub
  source_variable = x
  to_solution_tag = tagged_aux_sol
  variable = force
[]

[AuxVariables/x]
  initial_condition = 1
[]

[Executioner]
  type = Steady
[]
