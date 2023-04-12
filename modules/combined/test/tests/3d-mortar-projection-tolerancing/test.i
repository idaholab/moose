stress_free_temperature = 300
thermal_expansion_coeff = 6.66e-6

[Problem]
  type = FEProblem
[]

[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
  temperature = T_K
[]

[Mesh]
  patch_update_strategy = iteration
  use_displaced_mesh = true
  patch_size = 40
  [ori]
    type = FileMeshGenerator
    file = 'test.msh'
  []
[]

[Variables]
  [disp_x]
    block = 'pellet_inner pellet_outer'
  []
  [disp_y]
    block = 'pellet_inner pellet_outer'
  []
  [disp_z]
    block = 'pellet_inner pellet_outer'
  []
  [T_K]
    [InitialCondition]
      type = ConstantIC
      value = 300.0
    []
  []
  [lm_pellet]
    block = 'pellet_secondary_subdomain'
  []
[]

[Kernels]
  [solid_x]
    type = ADStressDivergenceTensors
    variable = disp_x
    component = 0
    block = 'pellet_inner pellet_outer'
    use_displaced_mesh = false
  []
  [solid_y]
    type = ADStressDivergenceTensors
    variable = disp_y
    component = 1
    block = 'pellet_inner pellet_outer'
    use_displaced_mesh = false
  []
  [solid_z]
    type = ADStressDivergenceTensors
    variable = disp_z
    component = 2
    block = 'pellet_inner pellet_outer'
    use_displaced_mesh = false
  []
  [timeder]
    type = ADHeatConductionTimeDerivative
    variable = 'T_K'
    density_name = density
    specific_heat = specific_heat
    block = 'pellet_inner pellet_outer'
    use_displaced_mesh = true
  []
  [diff]
    type = ADHeatConduction
    variable = 'T_K'
    thermal_conductivity = thermal_conductivity
    block = 'pellet_inner pellet_outer'
    use_displaced_mesh = true
  []
  [heatsource]
    type = ADMatHeatSource
    variable = 'T_K'
    material_property = radial_source
    block = 'pellet_inner pellet_outer'
    use_displaced_mesh = true
  []
[]

[Debug]
  show_var_residual_norms = TRUE
[]

[BCs]
  [mirror_z]
    type = ADDirichletBC
    variable = disp_z
    boundary = 'mirror_innerp mirror_outerp'
    value = 0
  []
  [mirror_x]
    type = ADDirichletBC
    variable = disp_x
    boundary = 'mirror_innerp mirror_outerp'
    value = 0
  []
  [mirror_y]
    type = ADDirichletBC
    variable = disp_y
    boundary = 'mirror_innerp mirror_outerp'
    value = 0
  []
[]

[Materials]
  [pellet_properties]
    type = ADGenericConstantMaterial
    prop_names = 'density  thermal_conductivity specific_heat'
    prop_values = '3.3112e3  34 1.2217e3'
    block = 'pellet_inner pellet_outer'
  []
  [pulse_shape_linear]
    type = ADGenericFunctionMaterial
    prop_values = '5e10*max(11455*(t)/7,1e-9)'
    prop_names = 'radial_source'
    output_properties = 'radial_source'
    block = 'pellet_inner pellet_outer'
    use_displaced_mesh = false
  []
  [strain]
    type = ADComputeSmallStrain
    displacements = 'disp_x disp_y disp_z'
    eigenstrain_names = eigenstrain #nameS!
    block = 'pellet_inner pellet_outer'
  []
  [thermal_strain]
    type = ADComputeThermalExpansionEigenstrain
    stress_free_temperature = ${stress_free_temperature}
    thermal_expansion_coeff = ${thermal_expansion_coeff}
    eigenstrain_name = eigenstrain
    block = 'pellet_inner pellet_outer'
  []
  [elasticity]
    type = ADComputeIsotropicElasticityTensor
    youngs_modulus = 3.306e11
    poissons_ratio = 0.329
  []
  [stress]
    type = ADComputeLinearElasticStress
    block = 'pellet_inner pellet_outer'
  []
[]

[Contact]
  [pellet]
    primary = void_pellet_0
    secondary = void_pellet_1
    model = frictionless
    formulation = mortar
    c_normal = 1e6
    correct_edge_dropping = true
  []
[]

[UserObjects]
  [conduction]
    type = GapFluxModelConduction
    temperature = T_K
    boundary = 'void_pellet_0 void_pellet_1'
    gap_conductivity = 0.4
    use_displaced_mesh = true
  []
  [rad_pellet]
    type = GapFluxModelRadiation
    temperature = T_K
    boundary = void_pellet_0
    primary_emissivity = 0.37
    secondary_emissivity = 0.37
    use_displaced_mesh = true
  []
[]

[Constraints]
  [gap_pellet]
    type = ModularGapConductanceConstraint
    variable = lm_pellet
    secondary_variable = T_K
    primary_boundary = 'void_pellet_0'
    primary_subdomain = pellet_primary_subdomain
    secondary_boundary = 'void_pellet_1'
    secondary_subdomain = pellet_secondary_subdomain
    gap_flux_models = 'conduction rad_pellet' #closed_pellet
    gap_geometry_type = 'CYLINDER'
    cylinder_axis_point_1 = '0 0 0'
    cylinder_axis_point_2 = '0 0 1'
    use_displaced_mesh = true
    quadrature = SECOND
  []
[]

[Executioner]
  type = Transient
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package -pc_factor_shift_type'
  petsc_options_value = 'lu       superlu_dist                  NONZERO'
  automatic_scaling = true
  line_search = none
  ignore_variables_for_autoscaling = 'pellet_normal_lm'
  compute_scaling_once = true
  scaling_group_variables = 'disp_x disp_y disp_z; T_K'
  nl_rel_tol = 1e-50
  nl_abs_tol = 1e-8
  nl_max_its = 20
  dtmin = 1e-3
  dt = 1e-3
  start_time = 0e-3
  end_time = 1
[]

[Outputs]
  [exodus]
    type = Exodus
    file_base = constMat
  []
  print_linear_residuals = false
[]
