[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = -1
  ymin = -1
  xmax = 1
  ymax = 1
  nx = 2
  ny = 2
  elem_type = QUAD4
[]

[MeshModifiers]
  [./subdomain_id]
    type = AssignElementSubdomainID
    subdomain_ids = '0 1
                     1 1'
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
  [./interface_average]
    type = InterfaceUO
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'interface'
    execute_on = ' timestep_end'
    average_type = average
  [../]
  [./interface_master_minus_slave]
    type = InterfaceUO
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'interface'
    execute_on = ' timestep_end'
    average_type = jump_master_minus_slave
  [../]
  [./interface_slave_minus_master]
    type = InterfaceUO
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'interface'
    execute_on = ' timestep_end'
    average_type = jump_slave_minus_master
  [../]
  [./interface_absolute_jump]
    type = InterfaceUO
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'interface'
    execute_on = ' timestep_end'
    average_type = jump_abs
  [../]
  [./interface_master]
    type = InterfaceUO
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'interface'
    execute_on = ' timestep_end'
    average_type = master
  [../]
  [./interface_slave]
    type = InterfaceUO
    var = diffusivity_1
    var_neighbor = diffusivity_2
    boundary = 'interface'
    execute_on = ' timestep_end'
    average_type = slave
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

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
