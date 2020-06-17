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
  dt = 0.1
  num_steps = 3
  dtmin = 0.1
  line_search = none
[]

[Outputs]
  [./out]
    type = Exodus
    sync_only = true
    sync_times = '0.1 0.2 0.3'
    execute_on = 'TIMESTEP_END'
  []
[]

[UserObjects]
  [./interface_value_uo]
    type = InterfaceQpValueUserObject
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'master0_interface'
    execute_on = 'initial timestep_end'
    interface_value_type = average
  [../]
  [./interface_master_minus_secondary_uo]
    type = InterfaceQpValueUserObject
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'master0_interface'
    execute_on = 'initial timestep_end'
    interface_value_type = jump_master_minus_secondary
  [../]
  [./interface_secondary_minus_master_uo]
    type = InterfaceQpValueUserObject
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'master0_interface'
    execute_on = 'initial timestep_end'
    interface_value_type = jump_secondary_minus_master
  [../]
  [./interface_absolute_jump_uo]
    type = InterfaceQpValueUserObject
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'master0_interface'
    execute_on = 'initial timestep_end'
    interface_value_type = jump_abs
  [../]
  [./interface_master_uo]
    type = InterfaceQpValueUserObject
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'master0_interface'
    execute_on = 'initial timestep_end'
    interface_value_type = master
  [../]
  [./interface_secondary_uo]
    type = InterfaceQpValueUserObject
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'master0_interface'
    execute_on = 'initial timestep_end'
    interface_value_type = secondary
  [../]
[]


[Materials]
  [./stateful1]
    type = StatefulMaterial
    block = 0
    initial_diffusivity = 5
  [../]
  [./stateful2]
    type = StatefulMaterial
    block = 1
    initial_diffusivity = 2
  [../]
[]

[AuxKernels]
  [./diffusivity_1]
    type = MaterialRealAux
    property = diffusivity
    variable = diffusivity_1
    execute_on = 'INITIAL  NONLINEAR'
  []
  [./diffusivity_2]
    type = MaterialRealAux
    property = diffusivity
    variable = diffusivity_2
    execute_on = 'INITIAL NONLINEAR'
  []
  [./interface_avg_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = avg_qp
    boundary = 'master0_interface'
    interface_uo_name = interface_value_uo
    execute_on = 'INITIAL TIMESTEP_END'
  []
  [./interface_master_minus_secondary_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = master_minus_secondary_qp
    boundary = 'master0_interface'
    interface_uo_name = interface_master_minus_secondary_uo
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./interface_secondary_minus_master_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = secondary_minus_master_qp
    boundary = 'master0_interface'
    interface_uo_name = interface_secondary_minus_master_uo
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./interface_absolute_jump_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = abs_jump_qp
    boundary = 'master0_interface'
    interface_uo_name = interface_absolute_jump_uo
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./interface_master_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = master_qp
    boundary = 'master0_interface'
    interface_uo_name = interface_master_uo
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./interface_secondary_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = secondary_qp
    boundary = 'master0_interface'
    interface_uo_name = interface_secondary_uo
    execute_on = 'INITIAL TIMESTEP_END'
  [../]


[]

[AuxVariables]
  [./diffusivity_1]
    family = MONOMIAL
    order = CONSTANT
  []
  [./diffusivity_2]
    family = MONOMIAL
    order = CONSTANT
  []
  [./avg_qp]
    family = MONOMIAL
    order = CONSTANT
  []
  [./master_minus_secondary_qp]
    family = MONOMIAL
    order = CONSTANT
  []
  [./secondary_minus_master_qp]
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
  [./secondary_qp]
    family = MONOMIAL
    order = CONSTANT
  []
[]



[Postprocessors]
  [./interface_average_PP]
    type = SideAverageValue
    boundary = 'master0_interface'
    variable =  avg_qp
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./master_minus_secondary_qp_PP]
    type = SideAverageValue
    boundary = 'master0_interface'
    variable =  master_minus_secondary_qp
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./secondary_minus_master_qp_PP]
    type = SideAverageValue
    boundary = 'master0_interface'
    variable =  secondary_minus_master_qp
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./abs_jump_qp_PP]
    type = SideAverageValue
    boundary = 'master0_interface'
    variable =  abs_jump_qp
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./master_qp_PP]
    type = SideAverageValue
    boundary = 'master0_interface'
    variable =  master_qp
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
  [./secondary_qp_PP]
    type = SideAverageValue
    boundary = 'master0_interface'
    variable =  secondary_qp
    execute_on = 'INITIAL TIMESTEP_END'
  [../]
[]
