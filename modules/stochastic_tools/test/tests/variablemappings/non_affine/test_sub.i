S = 10
D = 10
L = 5
P = 5

[Mesh]
  type = GeneratedMesh
  dim = 2
  ny = 500
  nx = 500
  xmax = ${L}
[]

[Problem]
  solve = false
[]

[Variables]
  [u]
  []
[]

[Kernels]
  [diffusion_u]
    type = ADMatDiffusion
    variable = u
    diffusivity = D_u
  []
  [source_u]
    type = ADBodyForce
    variable = u
    function = '1 + ${P} * cos(2*pi*x)^2 * cos(2*pi*y)^2'
  []
[]

[Functions]
  [du]
    type = ParsedFunction
    expression = '1 + D * sin(2*pi*x)^2 * sin(2*pi*y)^2 + ${S}'
    symbol_names = D
    symbol_values = ${D}
  []
[]

[Materials]
  [diffusivity_u]
    type = ADGenericFunctionMaterial
    prop_names = D_u
    prop_values = du
  []
[]

[BCs]
  # [left_u]
  #   type = DirichletBC
  #   variable = u
  #   boundary = left
  #   value = 0
  #   preset = true
  # []
  [right_u]
    type = ADDirichletBC
    variable = u
    boundary = right
    value = 0
    preset = true
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
[]

[VariableMappings]
  [rb_mapping]
    type = DEIMRBMapping
    filename = 'parallel_storage_main_mapping_rb_mapping.rd'
  []
[]

[UserObjects]
  [im]
    type = InverseRB
    mapping = rb_mapping
    execute_on = 'INITIAL TIMESTEP_BEGIN'
    force_preic = true
    max_iter = 1
  []
[]

[Outputs]
  exodus = true
[]
[AuxVariables]
  [on_node]
    initial_condition = 1.0
  []
  [on_elem]
    initial_condition = 1.0
    family = MONOMIAL
    order = CONSTANT
  []
[]

