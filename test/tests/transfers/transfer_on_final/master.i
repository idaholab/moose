[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
[]

[Variables]
  [u]
    initial_condition = 1234
  []
  [v]
    initial_condition = 2458
  []
[]

[Problem]
  type = FEProblem
  solve = false
[]

[Executioner]
  type = Transient
  num_steps = 4
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[MultiApps]
  [sub]
    type = TransientMultiApp
    input_files = sub.i
    execute_on = 'INITIAL TIMESTEP_END'
  []
[]

[Transfers]
  [from_sub]
    type = MultiAppCopyTransfer
    source_variable = u
    variable = u
    from_multi_app = sub
    check_multiapp_execute_on = false
    execute_on = 'FINAL'
  []
  [to_sub]
    type = MultiAppCopyTransfer
    source_variable = v
    variable = v
    to_multi_app = sub
    check_multiapp_execute_on = false
    execute_on = 'FINAL'
  []
[]

[Outputs]
  exodus = true
  [final]
    type = Exodus
    execute_on = 'FINAL'
    execute_input_on = 'NONE' # This is needed to avoid problems with creating a file w/o data during --recover testing
  []
[]
