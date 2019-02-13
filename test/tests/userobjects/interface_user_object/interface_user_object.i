[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  xmax = 2
  ny = 2
  ymax = 2
  elem_type = QUAD4
[]

[MeshModifiers]
  [./subdomain1]
    type = SubdomainBoundingBox
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
  [../]
  [./interface]
    type = SideSetsBetweenSubdomains
    depends_on = subdomain1
    master_block = '0'
    paired_block = '1'
    new_boundary = 'master0_interface'
  [../]
  [./break_boundary]
    depends_on = interface
    type = BreakBoundaryOnSubdomain
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
    D = 4
    block = 0
  [../]
  [./diff_v]
    type = CoeffParamDiffusion
    variable = v
    D = 2
    block = 1
  [../]
  [./source_u]
    type = BodyForce
    variable = u
    value = 0
  [../]
[]

[InterfaceKernels]
  [./interface]
    type = PenaltyInterfaceDiffusion
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


[UserObjects]
  [./interface_average]
    type = InterfaceUO
    var = u
    var_neighbor = v
    boundary = 'master0_interface'
    execute_on = 'initial timestep_end'
    average_type = average
  [../]
  [./interface_master_minus_slave]
    type = InterfaceUO
    var = u
    var_neighbor = v
    boundary = 'master0_interface'
    execute_on = 'initial timestep_end'
    average_type = jump_master_minus_slave
  [../]
  [./interface_slave_minus_master]
    type = InterfaceUO
    var = u
    var_neighbor = v
    boundary = 'master0_interface'
    execute_on = 'initial timestep_end'
    average_type = jump_slave_minus_master
  [../]
  [./interface_absolute_jump]
    type = InterfaceUO
    var = u
    var_neighbor = v
    boundary = 'master0_interface'
    execute_on = 'initial timestep_end'
    average_type = jump_abs
  [../]
  [./interface_master]
    type = InterfaceUO
    var = u
    var_neighbor = v
    boundary = 'master0_interface'
    execute_on = 'initial timestep_end'
    average_type = master
  [../]
  [./interface_slave]
    type = InterfaceUO
    var = u
    var_neighbor = v
    boundary = 'master0_interface'
    execute_on = 'initial timestep_end'
    average_type = slave
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
[]

[Postprocessors]
  [./interface_average_PP]
    type = InterfaceUOPPS
    user_object = interface_average
  [../]
  [./interface_master_minus_slave_PP]
    type = InterfaceUOPPS
    user_object = interface_master_minus_slave
  [../]
  [./interface_average_slave_minus_master_PP]
    type = InterfaceUOPPS
    user_object = interface_slave_minus_master
  [../]
  [./interface_average_absolute_jump_PP]
    type = InterfaceUOPPS
    user_object = interface_absolute_jump
  [../]
  [./interface_average_master_PP]
    type = InterfaceUOPPS
    user_object = interface_master
  [../]
  [./interface_average_slave_PP]
    type = InterfaceUOPPS
    user_object = interface_slave
  [../]
[]
