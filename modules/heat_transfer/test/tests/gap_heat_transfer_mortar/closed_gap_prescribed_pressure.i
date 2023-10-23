## Units in the input file: m-Pa-s-K

[Mesh]
  [left_rectangle]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 40
    ny = 10
    xmax = 1
    ymin = 0
    ymax = 0.5
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
    nx = 40
    ny = 10
    xmin = 1
    xmax = 2
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

[Variables]
  [temperature]
    initial_condition = 525.0
  []
  [temperature_interface_lm]
    block = 'interface_secondary_subdomain'
  []
[]

[AuxVariables]
  [interface_normal_lm]
    order = FIRST
    family = LAGRANGE
    block = 'interface_secondary_subdomain'
    initial_condition = 100.0
  []
[]

[Kernels]
  [HeatDiff_steel]
    type = ADHeatConduction
    variable = temperature
    thermal_conductivity = steel_thermal_conductivity
    block = 'left_block'
  []
  [HeatDiff_aluminum]
    type = ADHeatConduction
    variable = temperature
    thermal_conductivity = aluminum_thermal_conductivity
    block = 'right_block'
  []
[]

[BCs]
  [temperature_left]
    type = ADDirichletBC
    variable = temperature
    value = 800
    boundary = 'moving_block_left'
  []
  [temperature_right]
    type = ADDirichletBC
    variable = temperature
    value = 250
    boundary = 'fixed_block_right'
  []
[]

[Constraints]
  [thermal_contact]
    type = ModularGapConductanceConstraint
    variable = temperature_interface_lm
    secondary_variable = temperature
    primary_boundary = moving_block_right
    primary_subdomain = interface_primary_subdomain
    secondary_boundary = fixed_block_left
    secondary_subdomain = interface_secondary_subdomain
    gap_flux_models = 'closed'
  []
[]

[Materials]
  [steel_thermal_properties]
    type = ADGenericConstantMaterial
    prop_names = 'steel_density steel_thermal_conductivity steel_hardness'
    prop_values = '8e3            16.2                       129' ## for stainless steel 304
    block = 'left_block'
  []

  [aluminum_thermal_properties]
    type = ADGenericConstantMaterial
    prop_names = 'aluminum_density aluminum_thermal_conductivity aluminum_hardness'
    prop_values = ' 2.7e3           210                             15' #for 99% pure Al
    block = 'right_block'
  []
[]

[UserObjects]
  [closed]
    type = GapFluxModelPressureDependentConduction
    primary_conductivity = steel_thermal_conductivity
    secondary_conductivity = aluminum_thermal_conductivity
    temperature = temperature
    contact_pressure = interface_normal_lm
    primary_hardness = steel_hardness
    secondary_hardness = aluminum_hardness
    boundary = moving_block_right
  []
[]

[Postprocessors]
  [steel_interface_temperature]
    type = AverageNodalVariableValue
    variable = temperature
    block = interface_primary_subdomain
  []
  [aluminum_interface_temperature]
    type = AverageNodalVariableValue
    variable = temperature
    block = interface_secondary_subdomain
  []
  [interface_heat_flux_steel]
    type = ADSideDiffusiveFluxAverage
    variable = temperature
    boundary = moving_block_right
    diffusivity = steel_thermal_conductivity
  []
  [interface_heat_flux_aluminum]
    type = ADSideDiffusiveFluxAverage
    variable = temperature
    boundary = fixed_block_left
    diffusivity = aluminum_thermal_conductivity
  []
[]


[Executioner]
  type = Steady
  solve_type = NEWTON
  automatic_scaling = false

  nl_rel_tol = 1e-14
  nl_max_its = 20
[]

[Outputs]
  csv = true
  perf_graph = true
[]
