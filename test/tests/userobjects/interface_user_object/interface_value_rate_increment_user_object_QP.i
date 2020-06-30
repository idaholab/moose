[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    xmax = 2
    ny = 2
    ymax = 2
  []
  [./subdomain1]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
  [../]
  [./primary0_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain1
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'primary0_interface'
  [../]
  [./break_boundary]
    input = primary0_interface
    type = BreakBoundaryOnSubdomainGenerator
  [../]
[]

[Variables]
  [./u]
    order = FIRST
    family = LAGRANGE
    block = 0
  [../]

  [./v]
    order = FIRST
    family = LAGRANGE
    block = 1
  [../]
[]


[Kernels]
  [./diff_u]
    type = CoeffParamDiffusion
    variable = u
    D = 2
    block = 0
  [../]
  [./diff_v]
    type = CoeffParamDiffusion
    variable = v
    D = 4
    block = 1
  [../]
  [./source_u]
    type = BodyForce
    variable = u
    function = 0.1*t
  [../]
[]

[InterfaceKernels]
  [./primary0_interface]
    type = PenaltyInterfaceDiffusionDot
    variable = u
    neighbor_var = v
    boundary = primary0_interface
    penalty = 1e6
  [../]
[]

[BCs]
  [./u]
    type = VacuumBC
    variable = u
    boundary = 'left_to_0 bottom_to_0 right top'
  [../]
  [./v]
    type = VacuumBC
    variable = v
    boundary = 'left_to_1 bottom_to_1'
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = TRUE
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  dt = 0.1
  num_steps = 3
  dtmin = 0.1
  line_search = none
[]

[Outputs]
  exodus = true
[]

[UserObjects]
  [./interface_avg_value_uo]
    type = InterfaceQpValueUserObject
    var = v1
    var_neighbor = v2
    boundary = 'primary0_interface'
    execute_on = 'INITIAL LINEAR NONLINEAR TIMESTEP_BEGIN TIMESTEP_END FINAL'
    interface_value_type = average
  [../]
  [./interface_avg_value_rate_uo]
    type = InterfaceQpValueUserObject
    var = v1
    var_neighbor = v2
    boundary = 'primary0_interface'
    execute_on = 'INITIAL LINEAR NONLINEAR TIMESTEP_BEGIN TIMESTEP_END FINAL'
    interface_value_type = average
    value_type = rate
  [../]
  [./interface_avg_value_increment_uo]
    type = InterfaceQpValueUserObject
    var = v1
    var_neighbor = v2
    boundary = 'primary0_interface'
    execute_on = 'INITIAL LINEAR NONLINEAR TIMESTEP_BEGIN TIMESTEP_END FINAL'
    interface_value_type = average
    value_type = increment
  [../]
[]

[AuxKernels]
  [./v1_saux]
    type = StatefulAux
    coupled = v1
    variable = v1
  [../]
  [./v2_saux]
    type = StatefulAux
    coupled = v2
    variable = v2
  [../]
  [./interface_avg_value_aux]
    type = InterfaceValueUserObjectAux
    variable = avg_qp
    boundary = 'primary0_interface'
    interface_uo_name = interface_avg_value_uo
    execute_on = 'INITIAL LINEAR NONLINEAR TIMESTEP_BEGIN TIMESTEP_END FINAL'
  []
  [./interface_avg_value_rate_aux]
    type = InterfaceValueUserObjectAux
    variable = avg_rate_qp
    boundary = 'primary0_interface'
    interface_uo_name = interface_avg_value_rate_uo
    execute_on = 'INITIAL LINEAR NONLINEAR TIMESTEP_BEGIN TIMESTEP_END FINAL'
  []
  [./interface_avg_value_increment_aux]
    type = InterfaceValueUserObjectAux
    variable = avg_increment_qp
    boundary = 'primary0_interface'
    interface_uo_name = interface_avg_value_increment_uo
    execute_on = 'INITIAL LINEAR NONLINEAR TIMESTEP_BEGIN TIMESTEP_END FINAL'
  []
[]

[AuxVariables]
  [./v1]
    family = MONOMIAL
    order = FIRST
    initial_condition = 5
  [../]
  [./v2]
    family = MONOMIAL
    order = FIRST
    initial_condition = 2
  [../]
  [./avg_qp]
    family = MONOMIAL
    order = CONSTANT
  []
  [./avg_rate_qp]
    family = MONOMIAL
    order = CONSTANT
  []
  [./avg_increment_qp]
    family = MONOMIAL
    order = CONSTANT
  []
[]
