E_block = 1e7
E_plank = 1e7
elem = QUAD4
order = FIRST

[Mesh]
  patch_size = 80
  patch_update_strategy = auto
  [plank]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -0.3
    xmax = 0.3
    ymin = -10
    ymax = 10
    nx = 2
    ny = 67
    elem_type = ${elem}
    boundary_name_prefix = plank
  []
  [plank_id]
    type = SubdomainIDGenerator
    input = plank
    subdomain_id = 1
  []

  [block]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0.31
    xmax = 0.91
    ymin = 7.7
    ymax = 8.5
    nx = 3
    ny = 4
    elem_type = ${elem}
    boundary_name_prefix = block
    boundary_id_offset = 10
  []
  [block_id]
    type = SubdomainIDGenerator
    input = block
    subdomain_id = 2
  []

  [combined]
    type = MeshCollectionGenerator
    inputs = 'plank_id block_id'
  []
  [block_rename]
    type = RenameBlockGenerator
    input = combined
    old_block = '1 2'
    new_block = 'plank block'
  []

  [secondary]
    input = block_rename
    type = LowerDBlockFromSidesetGenerator
    sidesets = 'block_left'
    new_block_id = '30'
    new_block_name = 'frictionless_secondary_subdomain'
  []
  [primary]
    input = secondary
    type = LowerDBlockFromSidesetGenerator
    sidesets = 'plank_right'
    new_block_id = '20'
    new_block_name = 'frictionless_primary_subdomain'
  []
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Variables]
  [disp_x]
    order = ${order}
    block = 'plank block'
    scaling = '${fparse 2.0 / (E_plank + E_block)}'
  []
  [disp_y]
    order = ${order}
    block = 'plank block'
    scaling = '${fparse 2.0 / (E_plank + E_block)}'
  []
  [temp]
    order = ${order}
    block = 'plank block'
    scaling = 1e-1
  []
  [thermal_lm]
    order = ${order}
    block = 'frictionless_secondary_subdomain'
    scaling = 1e-7
  []
  [frictionless_normal_lm]
    order = ${order}
    block = 'frictionless_secondary_subdomain'
    use_dual = true
  []
[]

[AuxVariables]
  [stress_xx]
    order = FIRST
    family = MONOMIAL
    block = 'plank block'
  []
  [stress_yy]
    order = FIRST
    family = MONOMIAL
    block = 'plank block'
  []
  [stress_xx_recovered]
    order = FIRST
    family = LAGRANGE
    block = 'plank block'
  []
  [stress_yy_recovered]
    order = FIRST
    family = LAGRANGE
    block = 'plank block'
  []
[]

[AuxKernels]
  [stress_xx]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_xx
    index_i = 0
    index_j = 0
    execute_on = 'timestep_end'
    block = 'plank block'
  []
  [stress_yy]
    type = RankTwoAux
    rank_two_tensor = stress
    variable = stress_yy
    index_i = 1
    index_j = 1
    execute_on = 'timestep_end'
    block = 'plank block'
  []
  [stress_xx_recovered]
    type = NodalPatchRecoveryAux
    variable = stress_xx_recovered
    nodal_patch_recovery_uo = stress_xx_patch
    execute_on = 'TIMESTEP_END'
    block = 'plank block'
  []
  [stress_yy_recovered]
    type = NodalPatchRecoveryAux
    variable = stress_yy_recovered
    nodal_patch_recovery_uo = stress_yy_patch
    execute_on = 'TIMESTEP_END'
    block = 'plank block'
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [action]
    generate_output = 'stress_zz vonmises_stress hydrostatic_stress strain_xx strain_yy strain_zz'
    block = 'plank block'
    use_automatic_differentiation = false
    strain = FINITE
  []
[]

[Kernels]
  [hc]
    type = HeatConduction
    variable = temp
    use_displaced_mesh = true
    block = 'plank block'
  []
[]

[UserObjects]
  [weighted_gap_uo]
    type = LMWeightedGapUserObject
    primary_boundary = plank_right
    secondary_boundary = block_left
    primary_subdomain = frictionless_primary_subdomain
    secondary_subdomain = frictionless_secondary_subdomain
    lm_variable = frictionless_normal_lm
    disp_x = disp_x
    disp_y = disp_y
  []
  [stress_xx_patch]
    type = NodalPatchRecoveryMaterialProperty
    patch_polynomial_order = FIRST
    property = 'stress'
    component = '0 0'
    execute_on = 'NONLINEAR TIMESTEP_END'
    block = 'plank block'
  []
  [stress_yy_patch]
    type = NodalPatchRecoveryMaterialProperty
    patch_polynomial_order = FIRST
    property = 'stress'
    component = '1 1'
    execute_on = 'NONLINEAR TIMESTEP_END'
    block = 'plank block'
  []
[]

[Constraints]
  [weighted_gap_lm]
    type = ComputeWeightedGapLMMechanicalContact
    primary_boundary = plank_right
    secondary_boundary = block_left
    primary_subdomain = frictionless_primary_subdomain
    secondary_subdomain = frictionless_secondary_subdomain
    variable = frictionless_normal_lm
    disp_x = disp_x
    disp_y = disp_y
    use_displaced_mesh = true
    weighted_gap_uo = weighted_gap_uo
  []
  [normal_x]
    type = NormalMortarMechanicalContact
    primary_boundary = plank_right
    secondary_boundary = block_left
    primary_subdomain = frictionless_primary_subdomain
    secondary_subdomain = frictionless_secondary_subdomain
    variable = frictionless_normal_lm
    secondary_variable = disp_x
    component = x
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = weighted_gap_uo
  []
  [normal_y]
    type = NormalMortarMechanicalContact
    primary_boundary = plank_right
    secondary_boundary = block_left
    primary_subdomain = frictionless_primary_subdomain
    secondary_subdomain = frictionless_secondary_subdomain
    variable = frictionless_normal_lm
    secondary_variable = disp_y
    component = y
    use_displaced_mesh = true
    compute_lm_residuals = false
    weighted_gap_uo = weighted_gap_uo
  []
  [thermal_contact]
    type = GapConductanceConstraint
    variable = thermal_lm
    secondary_variable = temp
    k = 1
    use_displaced_mesh = true
    primary_boundary = plank_right
    primary_subdomain = frictionless_primary_subdomain
    secondary_boundary = block_left
    secondary_subdomain = frictionless_secondary_subdomain
    displacements = 'disp_x disp_y'
  []
[]

[BCs]
  [left_temp]
    type = DirichletBC
    variable = temp
    boundary = 'plank_left'
    value = 400
  []

  [right_temp]
    type = DirichletBC
    variable = temp
    boundary = 'block_right'
    value = 300
  []

  [left_x]
    type = DirichletBC
    variable = disp_x
    boundary = plank_left
    value = 0.0
  []
  [left_y]
    type = DirichletBC
    variable = disp_y
    boundary = plank_bottom
    value = 0.0
  []
  [right_x]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = block_right
    function = '-0.04*sin(4*(t+1.5))+0.02'
    preset = false
  []
  [right_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = block_right
    function = '-t'
    preset = false
  []
[]

[Materials]
  [plank]
    type = ComputeIsotropicElasticityTensor
    block = 'plank'
    poissons_ratio = 0.3
    youngs_modulus = ${E_plank}
  []
  [block]
    type = ComputeIsotropicElasticityTensor
    block = 'block'
    poissons_ratio = 0.3
    youngs_modulus = ${E_block}
  []
  [stress]
    type = ComputeFiniteStrainElasticStress
    block = 'plank block'
  []

  [heat_plank]
    type = HeatConductionMaterial
    block = plank
    thermal_conductivity = 2
    specific_heat = 1
  []
  [heat_block]
    type = HeatConductionMaterial
    block = block
    thermal_conductivity = 1
    specific_heat = 1
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  petsc_options = '-snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -snes_max_it'
  petsc_options_value = 'lu       NONZERO               1e-15                   20'
  end_time = 0.4
  dt = 0.1
  dtmin = 0.1
  timestep_tolerance = 1e-6
  line_search = 'none'
[]

[Postprocessors]
  [nl_its]
    type = NumNonlinearIterations
  []
  [total_nl_its]
    type = CumulativeValuePostprocessor
    postprocessor = nl_its
  []
  [l_its]
    type = NumLinearIterations
  []
  [total_l_its]
    type = CumulativeValuePostprocessor
    postprocessor = l_its
  []
  [contact]
    type = ContactDOFSetSize
    variable = frictionless_normal_lm
    subdomain = frictionless_secondary_subdomain
  []
  [avg_hydro]
    type = ElementAverageValue
    variable = hydrostatic_stress
    block = 'block'
  []
  [avg_temp]
    type = ElementAverageValue
    variable = temp
    block = 'block'
  []
  [max_hydro]
    type = ElementExtremeValue
    variable = hydrostatic_stress
    block = 'block'
  []
  [min_hydro]
    type = ElementExtremeValue
    variable = hydrostatic_stress
    block = 'block'
    value_type = min
  []
  [avg_vonmises]
    type = ElementAverageValue
    variable = vonmises_stress
    block = 'block'
  []
  [max_vonmises]
    type = ElementExtremeValue
    variable = vonmises_stress
    block = 'block'
  []
  [min_vonmises]
    type = ElementExtremeValue
    variable = vonmises_stress
    block = 'block'
    value_type = min
  []
  [stress_xx_recovered]
    type = ElementExtremeValue
    variable = stress_xx_recovered
    block = 'block'
    value_type = max
  []
  [stress_yy_recovered]
    type = ElementExtremeValue
    variable = stress_yy_recovered
    block = 'block'
    value_type = max
  []
[]

[Outputs]
  exodus = true
  [comp]
    type = CSV
    show = 'contact avg_temp'
  []
  [out]
    type = CSV
  []
  [dof]
    type = DOFMap
    execute_on = 'initial'
  []
[]

[Debug]
  show_var_residual_norms = true
[]
