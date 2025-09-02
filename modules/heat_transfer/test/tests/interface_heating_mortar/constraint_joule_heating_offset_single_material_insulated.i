## Units in the input file: m-Pa-s-K-V

[Mesh]
  [left_rectangle]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
    ny = 100
    xmax = 0.1
    ymin = 0.1
    ymax = 0.35
    boundary_name_prefix = moving_block
  []
  [left_block]
    type = SubdomainIDGenerator
    input = left_rectangle
    subdomain_id = 1
  []
  [right_rectangle]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 100
    ny = 100
    xmin = 0.1
    xmax = 0.2
    ymin = 0
    ymax = 0.5
    boundary_name_prefix = fixed_block
    boundary_id_offset = 4
  []
  [right_block]
    type = SubdomainIDGenerator
    input = right_rectangle
    subdomain_id = 2
  []
  [two_blocks]
    type = MeshCollectionGenerator
    inputs = 'left_block right_block'
  []
  [block_rename]
    type = RenameBlockGenerator
    input = two_blocks
    old_block = '1 2'
    new_block = 'left_block right_block'
  []
  [interface_secondary_subdomain]
    type = LowerDBlockFromSidesetGenerator
    sidesets = 'fixed_block_left'
    new_block_id = 3
    new_block_name = 'interface_secondary_subdomain'
    input = block_rename
  []
  [interface_primary_subdomain]
    type = LowerDBlockFromSidesetGenerator
    sidesets = 'moving_block_right'
    new_block_id = 4
    new_block_name = 'interface_primary_subdomain'
    input = interface_secondary_subdomain
  []
[]

[Problem]
  type = ReferenceResidualProblem
  reference_residual = 'ref'
  extra_tag_residuals = 'ref'
[]

[Variables]
  [temperature]
    initial_condition = 300.0
  []
  [potential]
  []
  [potential_interface_lm]
    block = 'interface_secondary_subdomain'
  []
[]

[AuxVariables]
  [interface_normal_lm]
    order = FIRST
    family = LAGRANGE
    block = 'interface_secondary_subdomain'
    initial_condition = 1.0
  []
[]

[Kernels]
  [HeatDiff_aluminum]
    type = ADHeatConduction
    variable = temperature
    thermal_conductivity = aluminum_thermal_conductivity
    extra_vector_tags = 'ref'
    block = 'left_block right_block'
  []
  [electric_aluminum]
    type = ADMatDiffusion
    variable = potential
    diffusivity = aluminum_electrical_conductivity
    extra_vector_tags = 'ref'
    block = 'left_block right_block'
  []
[]

[BCs]
  [temperature_left]
    type = ADDirichletBC
    variable = temperature
    value = 300
    boundary = 'moving_block_left'
  []
  [temperature_right]
    type = ADDirichletBC
    variable = temperature
    value = 300
    boundary = 'fixed_block_right'
  []

  [electric_left]
    type = ADDirichletBC
    variable = potential
    value = 0.0
    boundary = moving_block_left
  []
  [electric_right]
    type = ADDirichletBC
    variable = potential
    value = 3.0e-1
    boundary = fixed_block_right
  []
[]

[Constraints]
  [electrical_contact]
    type = ModularGapConductanceConstraint
    variable = potential_interface_lm
    secondary_variable = potential
    primary_boundary = moving_block_right
    primary_subdomain = interface_primary_subdomain
    secondary_boundary = fixed_block_left
    secondary_subdomain = interface_secondary_subdomain
    gap_flux_models = 'closed_electric'
    correct_edge_dropping = true
  []
  [interface_heating]
    type = ADInterfaceJouleHeatingConstraint
    potential_lagrange_multiplier = potential_interface_lm
    secondary_variable = temperature
    primary_electrical_conductivity = aluminum_electrical_conductivity
    secondary_electrical_conductivity = aluminum_electrical_conductivity
    primary_boundary = moving_block_right
    primary_subdomain = interface_primary_subdomain
    secondary_boundary = fixed_block_left
    secondary_subdomain = interface_secondary_subdomain
  []
[]

[Materials]
  [aluminum_thermal_properties]
    type = ADGenericConstantMaterial
    prop_names = 'aluminum_density aluminum_thermal_conductivity aluminum_heat_capacity aluminum_electrical_conductivity aluminum_hardness'
    prop_values = ' 2.7e3           210                           900.0                   3.7e7                           1.0' #for 99% pure Al
    block = 'left_block right_block interface_secondary_subdomain'
  []
[]

[UserObjects]
  [closed_electric]
    type = GapFluxModelPressureDependentConduction
    primary_conductivity = aluminum_electrical_conductivity
    secondary_conductivity = aluminum_electrical_conductivity
    temperature = potential
    contact_pressure = interface_normal_lm
    primary_hardness = aluminum_hardness
    secondary_hardness = aluminum_hardness
    boundary = moving_block_right
  []
[]

[Postprocessors]
  [interface_heat_flux_large_block]
    type = ADSideDiffusiveFluxAverage
    variable = temperature
    boundary = fixed_block_left
    diffusivity = aluminum_thermal_conductivity
  []
  [interface_heat_flux_small_block]
    type = ADSideDiffusiveFluxAverage
    variable = temperature
    boundary = moving_block_right
    diffusivity = aluminum_thermal_conductivity
  []
  [interface_electrical_flux_large_block]
    type = ADSideDiffusiveFluxAverage
    variable = potential
    boundary = fixed_block_left
    diffusivity = aluminum_electrical_conductivity
  []
  [interface_electrical_flux_small_block]
    type = ADSideDiffusiveFluxAverage
    variable = potential
    boundary = moving_block_right
    diffusivity = aluminum_electrical_conductivity
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  automatic_scaling = false
  line_search = 'none'

  nl_abs_tol = 2e-10
  nl_rel_tol = 1e-6
  nl_max_its = 50
  nl_forced_its = 1
[]

[Outputs]
  csv = true
  perf_graph = true
[]

