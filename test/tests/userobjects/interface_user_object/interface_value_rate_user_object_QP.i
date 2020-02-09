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
  [./master0_interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain1
    master_block = '0'
    paired_block = '1'
    new_boundary = 'master0_interface'
  [../]
  [./break_boundary]
    input = master0_interface
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
  [./master0_interface]
    type = PenaltyInterfaceDiffusionDot
    variable = u
    neighbor_var = v
    boundary = master0_interface
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

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = ' lu       superlu_dist                '
  dt = 1
  num_steps = 3
  dtmin = 1
  line_search = none
[]

[Outputs]
  exodus = true
[]

[UserObjects]
  [./interface_value_uo]
    type = InterfaceQpValueUserObject
    var = v1
    var_neighbor = v2
    boundary = 'master0_interface'
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
    interface_value_type = average
    compute_rate = true
  [../]
  [./interface_master_minus_slave_uo]
    type = InterfaceQpValueUserObject
    var = v1
    var_neighbor = v2
    boundary = 'master0_interface'
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
    interface_value_type = jump_master_minus_slave
    compute_rate = true
  [../]
  [./interface_slave_minus_master_uo]
    type = InterfaceQpValueUserObject
    var = v1
    var_neighbor = v2
    boundary = 'master0_interface'
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
    interface_value_type = jump_slave_minus_master
    compute_rate = true
  [../]
  [./interface_absolute_jump_uo]
    type = InterfaceQpValueUserObject
    var = v1
    var_neighbor = v2
    boundary = 'master0_interface'
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
    interface_value_type = jump_abs
    compute_rate = true
  [../]
  [./interface_master_uo]
    type = InterfaceQpValueUserObject
    var = v1
    var_neighbor = v2
    boundary = 'master0_interface'
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
    interface_value_type = master
    compute_rate = true
  [../]
  [./interface_slave_uo]
    type = InterfaceQpValueUserObject
    var = v1
    var_neighbor = v2
    boundary = 'master0_interface'
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
    interface_value_type = slave
    compute_rate = true
  [../]
[]

[AuxKernels]
  [./interface_avg_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = avg_qp
    boundary = 'master0_interface'
    interface_uo_name = interface_value_uo
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
    compute_rate = true
  []
  [./interface_master_minus_slave_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = master_minus_slave_qp
    boundary = 'master0_interface'
    interface_uo_name = interface_master_minus_slave_uo
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
    compute_rate = true
  [../]
  [./interface_slave_minus_master_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = slave_minus_master_qp
    boundary = 'master0_interface'
    interface_uo_name = interface_slave_minus_master_uo
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
    compute_rate = true
  [../]
  [./interface_absolute_jump_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = abs_jump_qp
    boundary = 'master0_interface'
    interface_uo_name = interface_absolute_jump_uo
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
    compute_rate = true
  [../]
  [./interface_master_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = master_qp
    boundary = 'master0_interface'
    interface_uo_name = interface_master_uo
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
    compute_rate = true
  [../]
  [./interface_slave_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = slave_qp
    boundary = 'master0_interface'
    interface_uo_name = interface_slave_uo
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
    compute_rate = true
  [../]


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
  [./master_minus_slave_qp]
    family = MONOMIAL
    order = CONSTANT
  []
  [./slave_minus_master_qp]
    family = MONOMIAL
    order = CONSTANT
  []
  [./abs_jump_qp]
    family = MONOMIAL
    order = CONSTANT
  []
  [./master_qp]
    family = MONOMIAL
    order = CONSTANT
  []
  [./slave_qp]
    family = MONOMIAL
    order = CONSTANT
  []
[]

[Postprocessors]
  [./interface_average_PP]
    type = SideAverageValue
    boundary = master0_interface
    variable =  avg_qp
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
  [../]
  [./master_minus_slave_qp_PP]
    type = SideAverageValue
    boundary = master0_interface
    variable =  master_minus_slave_qp
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
  [../]
  [./slave_minus_master_qp_PP]
    type = SideAverageValue
    boundary = master0_interface
    variable =  slave_minus_master_qp
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
  [../]
  [./abs_jump_qp_PP]
    type = SideAverageValue
    boundary = master0_interface
    variable =  abs_jump_qp
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
  [../]
  [./master_qp_PP]
    type = SideAverageValue
    boundary = master0_interface
    variable =  master_qp
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
  [../]
  [./slave_qp_PP]
    type = SideAverageValue
    boundary = master0_interface
    variable =  slave_qp
    execute_on = 'initial linear nonlinear timestep_begin timestep_end'
  [../]
[]
