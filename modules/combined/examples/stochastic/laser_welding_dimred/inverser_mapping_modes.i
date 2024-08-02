[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -.45e-3 # m
  xmax = 0.45e-3 # m
  ymin = -.9e-4 # m
  ymax = 0
  nx = 250
  ny = 45
  displacements = 'pod_disp_x pod_disp_y'
[]

[Variables]
  [vel]
    family = LAGRANGE_VEC
  []
  [T]
  []
  [T_pod]
  []
  [p]
  []
  [disp_x]
  []
  [pod_disp_x]
  []
  [disp_y]
  []
  [pod_disp_y]
  []
[]

[AuxVariables]
  [./vel_x_aux]
    [./InitialCondition]
      type = ConstantIC
      value = 1e-15
    [../]
  [../]
  [./vel_y_aux]
    [./InitialCondition]
      type = ConstantIC
      value = 1e-15
    [../]
  [../]
    [./vel_x_aux_pod]
    [../]
    [./vel_y_aux_pod]
    [../]
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
  [im_sol]
    type = InverseMapping
    mapping = pod_mapping_sol
    variable_to_fill = "T_pod pod_disp_x pod_disp_y"
    variable_to_reconstruct = "T disp_x disp_y"
    parameters = '1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
    execute_on = TIMESTEP_END
  []
  [im_aux]
    type = InverseMapping
    mapping = pod_mapping_aux
    variable_to_fill = "vel_x_aux_pod vel_y_aux_pod"
    variable_to_reconstruct = "vel_x_aux vel_y_aux"
    parameters = '1 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0'
    execute_on = TIMESTEP_END
  []
[]

[VariableMappings]
  [pod_mapping_sol]
    type = PODMapping
    filename = pod_mapping_train_mapping_sol_pod_mapping_sol.rd
    num_modes_to_compute = 2
  []
  [pod_mapping_aux]
    type = PODMapping
    filename = pod_mapping_train_mapping_aux_pod_mapping_aux.rd
    num_modes_to_compute = 2
  []
[]

[Outputs]
  exodus = true
  execute_on = 'FINAL'
[]
