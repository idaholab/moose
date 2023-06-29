[Mesh]
  second_order = true
  [MainMesh]
    type = GeneratedMeshGenerator
    dim = 1
    xmax = 1.0
    nx = 10
  []
[]

[AuxVariables]
  [Variable_1]
    order = SECOND
    family = LAGRANGE
  []
  [Variable_2]
    order = CONSTANT
    family = MONOMIAL
  []
  [Variable_3]
    order = SECOND
    family = LAGRANGE
  []
  [Variable_4]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[ICs]
  [Variable_1_IC]
    type = ConstantIC
    variable = Variable_1
    value = 1.0
  []
  [Variable_2_IC]
    type = ConstantIC
    variable = Variable_2
    value = 2.0
  []
[]

[UserObjects]
  [Variable_1_avg]
    type = NearestPointLayeredAverage
    direction = x
    num_layers = 10
    variable = Variable_1
    points = '0 0 0'
    execute_on = timestep_end
  []
  [Variable_2_avg]
    type = NearestPointLayeredAverage
    direction = x
    num_layers = 10
    variable = Variable_2
    points = '0 0 0'
    execute_on = timestep_end
  []
[]

[Problem]
  type = NoSolveProblem
[]

[Outputs]
  exodus = true
[]

[Executioner]
  type = Steady
[]

################################################################################
# Transfers
################################################################################
[MultiApps]
  [sub]
    type = FullSolveMultiApp
    input_files = "sub.i"
    execute_on = "timestep_end"
    positions = '0 0 0'
    output_in_position = true
    bounding_box_padding = '0.1 0.1 0.1'
  []
[]

[Transfers]
  [variable_1]
    type = MultiAppUserObjectTransfer2
    all_master_nodes_contained_in_sub_app = true
    multi_app = sub
    direction = to_multiapp
    variable = Variable_1
    user_object = Variable_1_avg
  []
  [variable_2]
    type = MultiAppUserObjectTransfer2
    all_master_nodes_contained_in_sub_app = true
    multi_app = sub
    direction = to_multiapp
    variable = Variable_2
    user_object = Variable_2_avg
  []

  [variable_3]
    type = MultiAppUserObjectTransfer2
    all_master_nodes_contained_in_sub_app = true
    multi_app = sub
    direction = from_multiapp
    variable = Variable_3
    user_object = Variable_3_avg
    execute_on = 'timestep_end'
  []
  [variable_4]
    type = MultiAppUserObjectTransfer2
    all_master_nodes_contained_in_sub_app = true
    multi_app = sub
    direction = from_multiapp
    variable = Variable_4
    user_object = Variable_4_avg
    execute_on = 'timestep_end'
  []
[]
