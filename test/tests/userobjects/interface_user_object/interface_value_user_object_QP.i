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
  [./subdomain_id]
    type = SubdomainBoundingBox
    bottom_left = '0 0 0'
    top_right = '1 1 0'
    block_id = 1
  [../]

  [./interface]
    type = SideSetsBetweenSubdomains
    depends_on = subdomain_id
    master_block = '0'
    paired_block = '1'
    new_boundary = 'interface'
  [../]

[]

[Functions]
  [./fn_exact]
    type = ParsedFunction
    value = 'x*x+y*y'
  [../]

  [./ffn]
    type = ParsedFunction
    value = -4
  [../]
[]

[UserObjects]
  [./interface_value_uo]
    type = InterfaceQpValueUserObject
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'interface'
    execute_on = 'initial timestep_end'
    interface_value_type = average
  [../]
  [./interface_master_minus_slave_uo]
    type = InterfaceQpValueUserObject
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'interface'
    execute_on = 'initial timestep_end'
    interface_value_type = jump_master_minus_slave
  [../]
  [./interface_slave_minus_master_uo]
    type = InterfaceQpValueUserObject
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'interface'
    execute_on = 'initial timestep_end'
    interface_value_type = jump_slave_minus_master
  [../]
  [./interface_absolute_jump_uo]
    type = InterfaceQpValueUserObject
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'interface'
    execute_on = 'initial timestep_end'
    interface_value_type = jump_abs
  [../]
  [./interface_master_uo]
    type = InterfaceQpValueUserObject
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'interface'
    execute_on = 'initial timestep_end'
    interface_value_type = master
  [../]
  [./interface_slave_uo]
    type = InterfaceQpValueUserObject
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'interface'
    execute_on = 'initial timestep_end'
    interface_value_type = slave
  [../]
[]

[Variables]
  [./u]
    family = LAGRANGE
    order = FIRST
  [../]
[]


[Kernels]
  [./diff]
    type = Diffusion
    variable = u
  [../]

  [./ffn]
    type = BodyForce
    variable = u
    function = ffn
  [../]
[]

[BCs]
  [./all]
    type = FunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = fn_exact
  [../]
[]

[Materials]
  [./stateful1]
    type = StatefulMaterial
    block = 0
    initial_diffusivity = 5
    # outputs = all
  [../]
  [./stateful2]
    type = StatefulMaterial
    block = 1
    initial_diffusivity = 2
    # outputs = all
  [../]
[]

[AuxKernels]
  [./diffusivity_1]
    type = MaterialRealAux
    property = diffusivity
    variable = diffusivity_1
  []
  [./diffusivity_2]
    type = MaterialRealAux
    property = diffusivity
    variable = diffusivity_2
  []
  [./interface_avg_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = avg_qp
    boundary = 'interface'
    interface_uo_name = interface_value_uo
  []
  [./interface_master_minus_slave_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = master_minus_slave_qp
    boundary = 'interface'
    interface_uo_name = interface_master_minus_slave_uo
  [../]
  [./interface_slave_minus_master_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = slave_minus_master_qp
    boundary = 'interface'
    interface_uo_name = interface_slave_minus_master_uo
  [../]
  [./interface_absolute_jump_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = abs_jump_qp
    boundary = 'interface'
    interface_uo_name = interface_absolute_jump_uo
  [../]
  [./interface_master_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = master_qp
    boundary = 'interface'
    interface_uo_name = interface_master_uo
  [../]
  [./interface_slave_qp_aux]
    type = InterfaceValueUserObjectAux
    variable = slave_qp
    boundary = 'interface'
    interface_uo_name = interface_slave_uo
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
    boundary = interface
    variable =  avg_qp
  [../]
  [./master_minus_slave_qp_PP]
    type = SideAverageValue
    boundary = interface
    variable =  master_minus_slave_qp
  [../]
  [./slave_minus_master_qp_PP]
    type = SideAverageValue
    boundary = interface
    variable =  slave_minus_master_qp
  [../]
  [./abs_jump_qp_PP]
    type = SideAverageValue
    boundary = interface
    variable =  abs_jump_qp
  [../]
  [./master_qp_PP]
    type = SideAverageValue
    boundary = interface
    variable =  master_qp
  [../]
  [./slave_qp_PP]
    type = SideAverageValue
    boundary = interface
    variable =  slave_qp
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
