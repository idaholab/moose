postprocessor_type = InterfaceAverageVariableValuePostprocessor

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 6
    xmax = 3
    ny = 9
    ymax = 3
    elem_type = QUAD4
  []
  [./subdomain_id]
    input = gen
    type = SubdomainBoundingBoxGenerator
    bottom_left = '0 0 0'
    top_right = '2 1 0'
    block_id = 1
    [../]
  [./interface]
    type = SideSetsBetweenSubdomainsGenerator
    input = subdomain_id
    primary_block = '0'
    paired_block = '1'
    new_boundary = 'interface'
  [../]
[]

[Functions]
  [./fn_exact]
    type = ParsedFunction
    expression = 'x*x+y*y'
  [../]

  [./ffn]
    type = ParsedFunction
    expression = -4
  [../]
[]

[Variables]
  [./u]
    family = MONOMIAL
    order = CONSTANT
    fv = true
  [../]
[]

[FVKernels]
  [./diff]
    type = FVDiffusion
    variable = u
    coeff = 1
  [../]

  [./ffn]
    type = FVBodyForce
    variable = u
    function = ffn
  [../]
[]

[FVBCs]
  [./all]
    type = FVFunctionDirichletBC
    variable = u
    boundary = '0 1 2 3'
    function = fn_exact
  [../]
[]

[Materials]
  [./stateful1]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'diffusivity'
    prop_values = 10
  [../]
  [./stateful2]
    type = GenericConstantMaterial
    block = 1
    prop_names = 'diffusivity'
    prop_values = 4
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
    type = ${postprocessor_type}
    interface_value_type = average
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./diffusivity_jump_primary_secondary]
    type = ${postprocessor_type}
    interface_value_type = jump_primary_minus_secondary
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./diffusivity_jump_secondary_primary]
    type = ${postprocessor_type}
    interface_value_type = jump_secondary_minus_primary
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./diffusivity_jump_abs]
    type = ${postprocessor_type}
    interface_value_type = jump_abs
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./diffusivity_primary]
    type = ${postprocessor_type}
    interface_value_type = primary
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./diffusivity_secondary]
    type = ${postprocessor_type}
    interface_value_type = secondary
    variable = diffusivity_1
    neighbor_variable = diffusivity_2
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
  [./diffusivity_single_variable]
    type = ${postprocessor_type}
    interface_value_type = primary
    variable = diffusivity_1
    execute_on = TIMESTEP_END
    boundary = 'interface'
  [../]
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[Outputs]
  file_base = ${raw ${postprocessor_type} _fv}
  exodus = true
[]
