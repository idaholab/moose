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
  [./diffusivity_average]
    type = InterfaceIntegralVariableAveragePostprocessor
    average_type = average
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./diffusivity_jump_master_slave]
    type = InterfaceIntegralVariableAveragePostprocessor
    average_type = jump_master_minus_slave
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./diffusivity_jump_slave_master]
    type = InterfaceIntegralVariableAveragePostprocessor
    average_type = jump_slave_minus_master
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./diffusivity_jump_abs]
    type = InterfaceIntegralVariableAveragePostprocessor
    average_type = jump_abs
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./diffusivity_master]
    type = InterfaceIntegralVariableAveragePostprocessor
    average_type = master
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./diffusivity_slave]
    type = InterfaceIntegralVariableAveragePostprocessor
    average_type = slave
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
