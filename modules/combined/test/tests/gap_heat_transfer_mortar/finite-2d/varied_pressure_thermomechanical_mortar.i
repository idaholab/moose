## Units in the input file: m-Pa-s-K

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Mesh]
  [left_rectangle]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 10
    xmax = 0.25
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
    nx = 20
    ny = 13
    xmin = 0.25
    xmax = 0.5
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
    initial_condition = 300.0
  []
  [temperature_interface_lm]
    block = 'interface_secondary_subdomain'
  []
[]

[Modules]
  [TensorMechanics/Master]
    [steel]
      strain = FINITE
      add_variables = false
      use_automatic_differentiation = true
      generate_output = 'strain_xx strain_xy strain_yy stress_xx stress_xy stress_yy'
      additional_generate_output = 'vonmises_stress'
      additional_material_output_family = 'MONOMIAL'
      additional_material_output_order = 'FIRST'
      block = 'left_block'
    []
    [aluminum]
      strain = FINITE
      add_variables = false
      use_automatic_differentiation = true
      generate_output = 'strain_xx strain_xy strain_yy stress_xx stress_xy stress_yy'
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
  [pressure_left_block]
    type = ADPressure
    variable = disp_x
    component = 0
    boundary = 'moving_block_left'
    function = '1e4*t*y'
  []
  [temperature_left]
    type = ADDirichletBC
    variable = temperature
    value = 300
    boundary = 'moving_block_left'
  []
  [temperature_right]
    type = ADDirichletBC
    variable = temperature
    value = 800
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
    type = ADComputeFiniteStrainElasticStress
    block = 'left_block'
  []
  [steel_thermal_properties]
    type = ADGenericConstantMaterial
    prop_names = 'steel_density steel_thermal_conductivity steel_heat_capacity steel_hardness'
    prop_values = ' 8e3            16.2                     0.5                 129' ## for stainless steel 304
    block = 'left_block'
  []

  [aluminum_elasticity_tensor]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 6.8e10 #in Pa, 68 GPa, aluminum
    poissons_ratio = 0.36
    block = 'right_block'
  []
  [aluminum_stress]
    type = ADComputeFiniteStrainElasticStress
    block = 'right_block'
  []
  [aluminum_thermal_properties]
    type = ADGenericConstantMaterial
    prop_names = 'aluminum_density aluminum_thermal_conductivity aluminum_heat_capacity aluminum_hardness'
    prop_values = ' 2.7e3            210                           0.9                   15' #for 99% pure Al
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
  [contact_pressure_max]
    type = NodalExtremeValue
    variable = interface_normal_lm
    block = interface_secondary_subdomain
    value_type = max
  []
  [contact_pressure_average]
    type = AverageNodalVariableValue
    variable = interface_normal_lm
    block = interface_secondary_subdomain
  []
  [contact_pressure_min]
    type = NodalExtremeValue
    variable = interface_normal_lm
    block = interface_secondary_subdomain
    value_type = min
  []
  [interface_temperature_max]
    type = NodalExtremeValue
    variable = temperature
    block = interface_secondary_subdomain
    value_type = max
  []
  [interface_temperature_average]
    type = AverageNodalVariableValue
    variable = temperature
    block = interface_secondary_subdomain
  []
  [interface_temperature_min]
    type = NodalExtremeValue
    variable = temperature
    block = interface_secondary_subdomain
    value_type = min
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

  nl_rel_tol = 1e-7
  nl_max_its = 20
  l_max_its = 50

  dt = 0.125
  end_time = 1
[]

[Outputs]
  csv = true
  perf_graph = true
[]
