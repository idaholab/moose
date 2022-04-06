[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [./u]
  [../]
[]

[ICs]
  [./u_ic]
    type = FunctionIC
    function = 'x*x*y'
    variable = u
  [../]
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 1
[]

[MultiApps]
  [./sub]
    type = FullSolveMultiApp
    input_files = 'sub.i'
    execute_on = timestep_end
  [../]
[]

[Transfers]
  [./to_sub]
    type = MultiAppCopyTransfer
    variable = u
    source_variable = u
    to_multi_app = sub
    execute_on = timestep_end
    check_multiapp_execute_on = false
  [../]
[]

[Outputs]
  exodus = true
[]
