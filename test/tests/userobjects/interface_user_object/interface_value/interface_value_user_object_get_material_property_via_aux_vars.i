[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 4
  xmax = 2
  ny = 6
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

# [UserObjects]
#   [./interface_value_type]
#     type = InterfaceValueAverageUO
#     var = diffusivity_1
#     var_neighbor = diffusivity_2
#     boundary = 'interface'
#     execute_on = ' timestep_end'
#     interface_value_type = average
#   [../]
#   [./interface_master_minus_slave]
#     type = InterfaceValueAverageUO
#     var = diffusivity_1
#     var_neighbor = diffusivity_2
#     boundary = 'interface'
#     execute_on = ' timestep_end'
#     interface_value_type = jump_master_minus_slave
#   [../]
#   [./interface_slave_minus_master]
#     type = InterfaceValueAverageUO
#     var = diffusivity_1
#     var_neighbor = diffusivity_2
#     boundary = 'interface'
#     execute_on = ' timestep_end'
#     interface_value_type = jump_slave_minus_master
#   [../]
#   [./interface_absolute_jump]
#     type = InterfaceValueAverageUO
#     var = diffusivity_1
#     var_neighbor = diffusivity_2
#     boundary = 'interface'
#     execute_on = ' timestep_end'
#     interface_value_type = jump_abs
#   [../]
#   [./interface_master]
#     type = InterfaceValueAverageUO
#     var = diffusivity_1
#     var_neighbor = diffusivity_2
#     boundary = 'interface'
#     execute_on = ' timestep_end'
#     interface_value_type = master
#   [../]
#   [./interface_slave]
#     type = InterfaceValueAverageUO
#     var = diffusivity_1
#     var_neighbor = diffusivity_2
#     boundary = 'interface'
#     execute_on = ' timestep_end'
#     interface_value_type = slave
#   [../]
# []

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
    block = 0
  []
  [./diffusivity_2]
    type = MaterialRealAux
    property = diffusivity
    variable = diffusivity_2
    block = 1
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
    type = InterfaceIntegralVariableAveragePostprocessor
    interface_value_type = average
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./interface_master_minus_slave_PP]
    type = InterfaceIntegralVariableAveragePostprocessor
    interface_value_type = jump_master_minus_slave
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./interface_average_slave_minus_master_PP]
    type = InterfaceIntegralVariableAveragePostprocessor
    interface_value_type = jump_slave_minus_master
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./interface_average_absolute_jump_PP]
    type = InterfaceIntegralVariableAveragePostprocessor
    interface_value_type = jump_abs
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./interface_average_master_PP]
    type = InterfaceIntegralVariableAveragePostprocessor
    interface_value_type = master
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./interface_average_slave_PP]
    type = InterfaceIntegralVariableAveragePostprocessor
    interface_value_type = slave
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  exodus = true
[]
