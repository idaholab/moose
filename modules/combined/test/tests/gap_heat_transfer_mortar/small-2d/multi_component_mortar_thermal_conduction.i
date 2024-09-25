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
    xmin = 1.
    xmax = 2.
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
  patch_update_strategy = iteration
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
  [HeatTdot_steel]
    type = ADHeatConductionTimeDerivative
    variable = temperature
    specific_heat = steel_heat_capacity
    density_name = steel_density
    block = 'left_block'
  []
  [HeatDiff_aluminum]
    type = ADHeatConduction
    variable = temperature
    thermal_conductivity = aluminum_thermal_conductivity
    block = 'right_block'
  []
  [HeatTdot_aluminum]
    type = ADHeatConductionTimeDerivative
    variable = temperature
    specific_heat = aluminum_heat_capacity
    density_name = aluminum_density
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
  [displacement_left_block]
    type = ADFunctionDirichletBC
    variable = disp_x
    function = 'if(t<61, 2.0e-7, -2.0e-8*(t-60))'
    boundary = 'moving_block_left'
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
    gap_flux_models = 'radiation closed'
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
  [steel_thermal_properties]
    type = ADGenericConstantMaterial
    prop_names = 'steel_density steel_thermal_conductivity steel_heat_capacity'
    prop_values = '  8e3          16.2                       0.5' ## for stainless steel 304
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
  [aluminum_thermal_properties]
    type = ADGenericConstantMaterial
    prop_names = 'aluminum_density aluminum_thermal_conductivity aluminum_heat_capacity'
    prop_values = ' 2.7e3            210                          0.9'
    block = 'right_block'
  []
[]

[UserObjects]
  [radiation]
    type = GapFluxModelRadiation
    secondary_emissivity = 0.25
    primary_emissivity = 0.6
    temperature = temperature
    boundary = moving_block_right
  []
  [closed]
    type = GapFluxModelPressureDependentConduction
    primary_conductivity = steel_thermal_conductivity
    secondary_conductivity = aluminum_thermal_conductivity
    temperature = temperature
    contact_pressure = interface_normal_lm
    primary_hardness = 1.0
    secondary_hardness = 1.0
    boundary = moving_block_right
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
  [aluminum_element_interface_stress]
    type = ElementalVariableValue
    variable = vonmises_stress
    elementid = 560
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
  type = Transient
  solve_type = NEWTON
  automatic_scaling = false
  line_search = 'none'

  # mortar contact solver options
  petsc_options = '-snes_converged_reason -pc_svd_monitor'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = ' lu       superlu_dist'
  snesmf_reuse_base = false

  nl_rel_tol = 1e-10
  nl_max_its = 20
  l_max_its = 50

  dt = 60
  end_time = 120
[]

[Outputs]
  csv = true
  perf_graph = true
[]
