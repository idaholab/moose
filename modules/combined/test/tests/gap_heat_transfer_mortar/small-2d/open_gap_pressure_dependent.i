## Units in the input file: m-Pa-s-K

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

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
    xmin = 1.0001
    xmax = 2.0001
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
[]

[Variables]
  [disp_x]
    block = 'left_block right_block'
  []
  [disp_y]
    block = 'left_block right_block'
  []
  [temperature]
    initial_condition = 525.0
  []
  [temperature_interface_lm]
    block = 'interface_secondary_subdomain'
  []
[]

[Physics]
  [SolidMechanics/QuasiStatic]
    [steel]
      strain = SMALL
      add_variables = false
      use_automatic_differentiation = true
      additional_generate_output = 'vonmises_stress'
      additional_material_output_family = 'MONOMIAL'
      additional_material_output_order = 'FIRST'
      block = 'left_block'
    []
    [aluminum]
      strain = SMALL
      add_variables = false
      use_automatic_differentiation = true
      additional_generate_output = 'vonmises_stress'
      additional_material_output_family = 'MONOMIAL'
      additional_material_output_order = 'FIRST'
      block = 'right_block'
    []
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
  [fixed_bottom_edge]
    type = ADDirichletBC
    variable = disp_y
    value = 0
    boundary = 'moving_block_bottom fixed_block_bottom'
  []
  [fixed_outer_edge]
    type = ADDirichletBC
    variable = disp_x
    value = 0
    boundary = 'fixed_block_right'
  []
  [pressure_left_block]
    type = ADPressure
    variable = disp_x
    boundary = 'moving_block_left'
    component = 0
    function = 1*t
  []
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

[Contact]
  [interface]
    primary = moving_block_right
    secondary = fixed_block_left
    model = frictionless
    formulation = mortar
    correct_edge_dropping = true
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
    use_displaced_mesh = true
  []
[]

[Materials]
  [steel_elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 1.93e11 #in Pa, 193 GPa, stainless steel 304
    poissons_ratio = 0.29
    block = 'left_block'
  []
  [steel_stress]
    type = ADComputeLinearElasticStress
    block = 'left_block'
  []
  [steel_density]
    type = ADGenericConstantMaterial
    prop_names = 'steel_density'
    prop_values = 8e3 #in kg/m^3, stainless steel 304
    block = 'left_block'
  []
  [steel_thermal_properties]
    type = ADGenericConstantMaterial
    prop_names = 'steel_thermal_conductivity steel_heat_capacity steel_emissivity'
    prop_values = '16.2 0.5 0.6' ## for stainless steel 304
    block = 'left_block'
  []

  [aluminum_elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 6.8e10 #in Pa, 68 GPa, aluminum
    poissons_ratio = 0.36
    block = 'right_block'
  []
  [aluminum_stress]
    type = ADComputeLinearElasticStress
    block = 'right_block'
  []
  [aluminum_density]
    type = ADGenericConstantMaterial
    prop_names = 'aluminum_density'
    prop_values = 2.7e3 #in kg/m^3, stainless steel 304
    block = 'right_block'
  []
  [aluminum_thermal_properties]
    type = ADGenericConstantMaterial
    prop_names = 'aluminum_thermal_conductivity aluminum_heat_capacity aluminum_emissivity'
    prop_values = '210 0.9 0.25'
    block = 'right_block'
  []
[]

[UserObjects]
  [closed]
    type = GapFluxModelPressureDependentConduction
    primary_conductivity = steel_thermal_conductivity
    secondary_conductivity = aluminum_thermal_conductivity
    temperature = temperature
    primary_hardness = 1.0
    secondary_hardness = 1.0
    boundary = moving_block_right
    contact_pressure = interface_normal_lm
  []
[]

[Postprocessors]
  [steel_pt_interface_temperature]
    type = NodalVariableValue
    nodeid = 245
    variable = temperature
  []
  [aluminum_pt_interface_temperature]
    type = NodalVariableValue
    nodeid = 657
    variable = temperature
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
  [steel_element_interface_stress]
    type = ElementalVariableValue
    variable = vonmises_stress
    elementid = 199
  []
  [aluminum_element_interface_stress]
    type = ElementalVariableValue
    variable = vonmises_stress
    elementid = 560
  []
[]


[Executioner]
  type = Steady
  solve_type = NEWTON
  automatic_scaling = false
  line_search = 'none'

  petsc_options_iname = '-pc_type'
  petsc_options_value = 'lu'
[]

[Outputs]
  csv = true
  perf_graph = true
[]
