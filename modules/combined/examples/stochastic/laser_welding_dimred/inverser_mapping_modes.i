thickness=0.9e-4 # m
xmin=-0.1e-3 # m
xmax=0.75e-3 # m
ymin=${fparse -thickness}

[Mesh]
  [cmg]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = ${xmin}
    xmax = ${xmax}
    ymin = ${fparse ymin}
    ymax = 0
    nx = 161
    ny = 50
  []
[]

[Variables]
  [T]
  []
  [T_pod1]
  []
  [T_pod2]
  []
  [T_pod3]
  []
  [T_pod4]
  []
  [T_pod5]
  []
  [T_pod6]
  []
  [T_pod7]
  []
  [T_pod8]
  []
  [T_pod9]
  []
  [T_pod10]
  []
[]

[Problem]
  solve = false
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_hypre_type'
  petsc_options_value = 'hypre boomeramg'
[]

[UserObjects]
  [mode1]
    type = InverseMapping
    mapping = pod_mapping_sol
    variable_to_fill = "T_pod1"
    variable_to_reconstruct = "T"
    parameters = '1 0 0 0 0 0 0 0 0 0'
    execute_on = TIMESTEP_END
  []
  [mode2]
    type = InverseMapping
    mapping = pod_mapping_sol
    variable_to_fill = "T_pod2"
    variable_to_reconstruct = "T"
    parameters = '0 1 0 0 0 0 0 0 0 0'
    execute_on = TIMESTEP_END
  []
  [mode3]
    type = InverseMapping
    mapping = pod_mapping_sol
    variable_to_fill = "T_pod3"
    variable_to_reconstruct = "T"
    parameters = '0 0 1 0 0 0 0 0 0 0'
    execute_on = TIMESTEP_END
  []
  [mode4]
    type = InverseMapping
    mapping = pod_mapping_sol
    variable_to_fill = "T_pod4"
    variable_to_reconstruct = "T"
    parameters = '0 0 0 1 0 0 0 0 0 0'
    execute_on = TIMESTEP_END
  []
  [mode5]
    type = InverseMapping
    mapping = pod_mapping_sol
    variable_to_fill = "T_pod5"
    variable_to_reconstruct = "T"
    parameters = '0 0 0 0 1 0 0 0 0 0'
    execute_on = TIMESTEP_END
  []
  [mode6]
    type = InverseMapping
    mapping = pod_mapping_sol
    variable_to_fill = "T_pod6"
    variable_to_reconstruct = "T"
    parameters = '0 0 0 0 0 1 0 0 0 0'
    execute_on = TIMESTEP_END
  []
  [mode7]
    type = InverseMapping
    mapping = pod_mapping_sol
    variable_to_fill = "T_pod7"
    variable_to_reconstruct = "T"
    parameters = '0 0 0 0 0 0 1 0 0 0'
    execute_on = TIMESTEP_END
  []
  [mode8]
    type = InverseMapping
    mapping = pod_mapping_sol
    variable_to_fill = "T_pod8"
    variable_to_reconstruct = "T"
    parameters = '0 0 0 0 0 0 0 1 0 0'
    execute_on = TIMESTEP_END
  []
  [mode9]
    type = InverseMapping
    mapping = pod_mapping_sol
    variable_to_fill = "T_pod8"
    variable_to_reconstruct = "T"
    parameters = '0 0 0 0 0 0 0 0 1 0'
    execute_on = TIMESTEP_END
  []
  [mode10]
    type = InverseMapping
    mapping = pod_mapping_sol
    variable_to_fill = "T_pod8"
    variable_to_reconstruct = "T"
    parameters = '0 0 0 0 0 0 0 0 0 1'
    execute_on = TIMESTEP_END
  []
[]

[VariableMappings]
  [pod_mapping_sol]
    type = PODMapping
    filename = pod_mapping_train_mapping_sol_pod_mapping_sol.rd
    num_modes_to_compute = 10
  []
[]

[Outputs]
  exodus = true
  execute_on = 'FINAL'
[]
