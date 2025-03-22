S = 10
D = 10
L = 5

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 100
  ny = 25
  nz = 25
  xmax = ${L}
[]

[Problem]
  solve = false
  boundary_restricted_node_integrity_check = false
  boundary_restricted_elem_integrity_check = false
  skip_nl_system_check = true
  kernel_coverage_check = false
  fv_bcs_integrity_check = false
  material_coverage_check = false
  material_dependency_check = false
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
    value = 1.0
  []
[]

[Functions]
  [du]
    type = ParsedFunction
    expression = 'D * D * x + 1'
    symbol_names = D
    symbol_values = ${D}
  []
[]

[Materials]
  [diffusivity_u]
    type = ADGenericFunctionMaterial
    prop_names = D_u
    prop_values = du
    compute = true
  []
[]

[BCs]
  [left_u]
    type = ADDirichletBC
    variable = u
    boundary = ' front top'
    value = 0
    preset = false
  []
  [right_u]
    type = ADDirichletBC
    variable = u
    boundary = right
    value = ${S}
    preset = false
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
  [pgraph]
    type = PerfGraphOutput
    execute_on = ' final' # Default is "final"
    level = 3 # Default is 1
    heaviest_sections = 10
  []
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
