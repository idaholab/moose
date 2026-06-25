AD = ''
use_ad = false
temp_scaling = 1
disp_x_scaling = ${fparse 1 / temp_scaling}

[GlobalParams]
  order = FIRST
  displacements = 'disp_x disp_y'
[]

[Problem]
  extra_tag_vectors = 'constraint_res kernel_res constraint_ref kernel_ref absolute_ref total_res'
[]

[Mesh]
  [gen]
    type = CartesianMeshGenerator
    dim = 2
    dx = '1 0.01 1'
    dy = 1
    ix = '5 1 5'
    iy = '20'
    subdomain_id = '0 1 2'
  []
  [rename]
    type = RenameBlockGenerator
    input = gen
    old_block = '0 1 2'
    new_block = 'a b c'
  []
  [sidesets_a]
    type = SideSetsBetweenSubdomainsGenerator
    input = rename
    new_boundary = a_right
    paired_block = b
    primary_block = a
  []
  [sidesets_c]
    type = SideSetsBetweenSubdomainsGenerator
    input = sidesets_a
    new_boundary = c_left
    paired_block = b
    primary_block = c
  []
  [delete]
    type = BlockDeletionGenerator
    input = sidesets_c
    block = b
  []

  patch_update_strategy = always
  coord_type = RZ
[]

[Variables]
  [temp]
    initial_condition = 298
    scaling = ${temp_scaling}
  []
  [disp_x]
    scaling = ${disp_x_scaling}
  []
  [disp_y]
  []
[]

[AuxVariables]
  [constraint_res_temp]
  []
  [constraint_res_disp_x]
  []
  [constraint_res_disp_y]
  []
  [kernel_res_temp]
  []
  [kernel_res_disp_x]
  []
  [kernel_res_disp_y]
  []

  [constraint_ref_temp]
  []
  [constraint_ref_disp_x]
  []
  [constraint_ref_disp_y]
  []
  [kernel_ref_temp]
  []
  [kernel_ref_disp_x]
  []
  [kernel_ref_disp_y]
  []

  [absolute_ref_temp]
  []
  [absolute_ref_disp_x]
  []
  [absolute_ref_disp_y]
  []
  [total_res_temp]
  []
  [total_res_disp_x]
  []
  [total_res_disp_y]
  []
[]

[AuxKernels]
  [constraint_res_temp_aux]
    type = TagVectorAux
    variable = constraint_res_temp
    v = temp
    vector_tag = constraint_res
  []
  [constraint_res_disp_x_aux]
    type = TagVectorAux
    variable = constraint_res_disp_x
    v = disp_x
    vector_tag = constraint_res
  []
  [constraint_res_disp_y_aux]
    type = TagVectorAux
    variable = constraint_res_disp_y
    v = disp_y
    vector_tag = constraint_res
  []
  [kernel_res_temp_aux]
    type = TagVectorAux
    variable = kernel_res_temp
    v = temp
    vector_tag = kernel_res
  []
  [kernel_res_disp_x_aux]
    type = TagVectorAux
    variable = kernel_res_disp_x
    v = disp_x
    vector_tag = kernel_res
  []
  [kernel_res_disp_y_aux]
    type = TagVectorAux
    variable = kernel_res_disp_y
    v = disp_y
    vector_tag = kernel_res
  []

  [constraint_ref_temp_aux]
    type = TagVectorAux
    variable = constraint_ref_temp
    v = temp
    vector_tag = constraint_ref
  []
  [constraint_ref_disp_x_aux]
    type = TagVectorAux
    variable = constraint_ref_disp_x
    v = disp_x
    vector_tag = constraint_ref
  []
  [constraint_ref_disp_y_aux]
    type = TagVectorAux
    variable = constraint_ref_disp_y
    v = disp_y
    vector_tag = constraint_ref
  []
  [kernel_ref_temp_aux]
    type = TagVectorAux
    variable = kernel_ref_temp
    v = temp
    vector_tag = kernel_ref
  []
  [kernel_ref_disp_x_aux]
    type = TagVectorAux
    variable = kernel_ref_disp_x
    v = disp_x
    vector_tag = kernel_ref
  []
  [kernel_ref_disp_y_aux]
    type = TagVectorAux
    variable = kernel_ref_disp_y
    v = disp_y
    vector_tag = kernel_ref
  []

  [absolute_ref_temp_aux]
    type = TagVectorAux
    variable = absolute_ref_temp
    v = temp
    vector_tag = absolute_ref
  []
  [absolute_ref_disp_x_aux]
    type = TagVectorAux
    variable = absolute_ref_disp_x
    v = disp_x
    vector_tag = absolute_ref
  []
  [absolute_ref_disp_y_aux]
    type = TagVectorAux
    variable = absolute_ref_disp_y
    v = disp_y
    vector_tag = absolute_ref
  []
  [total_res_temp_aux]
    type = TagVectorAux
    variable = total_res_temp
    v = temp
    vector_tag = total_res
  []
  [total_res_disp_x_aux]
    type = TagVectorAux
    variable = total_res_disp_x
    v = disp_x
    vector_tag = total_res
  []
  [total_res_disp_y_aux]
    type = TagVectorAux
    variable = total_res_disp_y
    v = disp_y
    vector_tag = total_res
  []
[]

[Physics/SolidMechanics/QuasiStatic]
  [all]
    block = 'a c'
    strain = FINITE
    eigenstrain_names = 'thermal_strain'
    temperature = temp
    use_automatic_differentiation = ${use_ad}
    volumetric_locking_correction = true
    extra_vector_tags = 'kernel_res total_res'
    absolute_value_vector_tags = 'absolute_ref kernel_ref'
  []
[]

[Kernels]
  [heat]
    type = ${AD}Diffusion
    block = 'a c'
    variable = temp
    extra_vector_tags = 'kernel_res total_res'
    absolute_value_vector_tags = 'absolute_ref kernel_ref'
  []
  [heat_ie]
    type = ${AD}HeatConductionTimeDerivative
    block = 'a c'
    variable = temp
    specific_heat = 1
    density_name = 1
    extra_vector_tags = 'kernel_res total_res'
    absolute_value_vector_tags = 'absolute_ref kernel_ref'
  []
  [heat_source]
    type = BodyForce
    block = 'a c'
    variable = temp
    value = 5e3
    extra_vector_tags = 'kernel_res total_res'
    absolute_value_vector_tags = 'absolute_ref kernel_ref'
  []
[]

[Contact]
  [frictional]
    model = coulomb
    # model = frictionless
    formulation = mortar
    primary = a_right
    secondary = c_left
    friction_coefficient = 0.2
    extra_vector_tags = 'constraint_res total_res'
    absolute_value_vector_tags = 'absolute_ref constraint_ref'
  []
[]

[MortarGapHeatTransfer]
  [mortar_heat_transfer]
    temperature = temp
    use_displaced_mesh = true
    gap_flux_options = conduction
    gap_conductivity = 1
    boundary = 'a_right c_left'
    primary_boundary = a_right
    secondary_boundary = c_left
    primary_subdomain = frictional_primary_subdomain
    secondary_subdomain = frictional_secondary_subdomain
    gap_geometry_type = CYLINDER
    primary_emissivity = 0
    secondary_emissivity = 0
    extra_vector_tags = 'constraint_res total_res'
    absolute_value_vector_tags = 'absolute_ref constraint_ref'
  []
[]

[BCs]
  [no_y_clad]
    type = ${AD}DirichletBC
    variable = disp_y
    boundary = 'bottom'
    value = 0.0
    preset = false
    extra_vector_tags = 'constraint_res total_res'
    absolute_value_vector_tags = 'absolute_ref constraint_ref'
  []
  [no_x_clad]
    type = ${AD}DirichletBC
    variable = disp_x
    boundary = 'right'
    value = 0.0
    preset = false
    extra_vector_tags = 'constraint_res total_res'
    absolute_value_vector_tags = 'absolute_ref constraint_ref'
  []
  [Pressure]
    [coolantPressure]
      boundary = 'a_right c_left'
      factor = 0.345e6
      use_automatic_differentiation = ${use_ad}
      extra_vector_tags = 'constraint_res total_res'
    absolute_value_vector_tags = 'absolute_ref constraint_ref'
    []
  []
  [convection]
    type = ConvectiveHeatFluxBC
    variable = temp
    boundary = right
    T_infinity = 298
    heat_transfer_coefficient = 1e-1
    extra_vector_tags = 'constraint_res total_res'
    absolute_value_vector_tags = 'absolute_ref constraint_ref'
  []
  [top]
    type = ${AD}DirichletBC
    variable = temp
    boundary = top
    value = 298
    extra_vector_tags = 'constraint_res total_res'
    absolute_value_vector_tags = 'absolute_ref constraint_ref'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ${AD}ComputeIsotropicElasticityTensor
    block = 'a c'
    poissons_ratio = 0.3
    youngs_modulus = 1e10
  []
  [thermal_expansion_a]
    type = ${AD}ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1e-5
    block = 'a'
    temperature = temp
    stress_free_temperature = 298.0
    eigenstrain_name = thermal_strain
  []
  [thermal_expansion_c]
    type = ${AD}ComputeThermalExpansionEigenstrain
    thermal_expansion_coeff = 1e-6
    block = 'c'
    temperature = temp
    stress_free_temperature = 298.0
    eigenstrain_name = thermal_strain
  []
  [stress]
    type = ${AD}ComputeFiniteStrainElasticStress
    block = 'a c'
  []
[]

[Convergence]
  [temp_ref_check]
    type = ReferenceResidualConvergence
    reference_vector = 'absolute_ref'
    converge_on = 'temp'
    nl_rel_tol = 5e-6
    nl_abs_tol = 5e-9
    nl_max_its = 20
    nl_div_tol = -1
    verbose = true
    unscale_the_residual = true
  []
  [disp_ref_check]
    type = ReferenceResidualConvergence
    reference_vector = 'absolute_ref'
    converge_on = 'disp_x disp_y'
    nl_rel_tol = 5e-6
    nl_abs_tol = 5e-9
    nl_max_its = 20
    nl_div_tol = -1
    verbose = true
    unscale_the_residual = true
  []
  [all_ref_conv]
    type = ParsedConvergence

    symbol_names = 'temp_ref_check disp_ref_check'
    symbol_values = 'temp_ref_check disp_ref_check'

    convergence_expression = 'temp_ref_check & disp_ref_check'
    divergence_expression = 'temp_ref_check | disp_ref_check'
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  nonlinear_convergence = all_ref_conv
  petsc_options = '-snes_ksp_ew -snes_converged_reason -ksp_converged_reason'
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount'
  petsc_options_value = 'lu NONZERO               1e-25'
  line_search = 'none'
  verbose = true

  l_max_its = 60

  num_steps = 9
  dt = 0.1
[]

[Postprocessors]
  [temp_avg]
    type = ElementAverageValue
    variable = temp
    outputs = console
  []
  [constraint_res_temp]
    type = NodalL2Norm
    variable = constraint_res_temp
    outputs = console
  []
  [constraint_res_disp_x]
    type = NodalL2Norm
    variable = constraint_res_disp_x
    outputs = console
  []
  [constraint_res_disp_y]
    type = NodalL2Norm
    variable = constraint_res_disp_y
    outputs = console
  []
  [kernel_res_temp]
    type = NodalL2Norm
    variable = kernel_res_temp
    outputs = console
  []
  [kernel_res_disp_x]
    type = NodalL2Norm
    variable = kernel_res_disp_x
    outputs = console
  []
  [kernel_res_disp_y]
    type = NodalL2Norm
    variable = kernel_res_disp_y
    outputs = console
  []

  [constraint_ref_temp]
    type = NodalL2Norm
    variable = constraint_ref_temp
    outputs = console
  []
  [constraint_ref_disp_x]
    type = NodalL2Norm
    variable = constraint_ref_disp_x
    outputs = console
  []
  [constraint_ref_disp_y]
    type = NodalL2Norm
    variable = constraint_ref_disp_y
    outputs = console
  []
  [kernel_ref_temp]
    type = NodalL2Norm
    variable = kernel_ref_temp
    outputs = console
  []
  [kernel_ref_disp_x]
    type = NodalL2Norm
    variable = kernel_ref_disp_x
    outputs = console
  []
  [kernel_ref_disp_y]
    type = NodalL2Norm
    variable = kernel_ref_disp_y
    outputs = console
  []

  [absolute_ref_temp]
    type = NodalL2Norm
    variable = absolute_ref_temp
    outputs = console
  []
  [absolute_ref_disp_x]
    type = NodalL2Norm
    variable = absolute_ref_disp_x
    outputs = console
  []
  [absolute_ref_disp_y]
    type = NodalL2Norm
    variable = absolute_ref_disp_y
    outputs = console
  []
  [total_res_temp]
    type = NodalL2Norm
    variable = total_res_temp
    outputs = console
  []
  [total_res_disp_x]
    type = NodalL2Norm
    variable = total_res_disp_x
    outputs = console
  []
  [total_res_disp_y]
    type = NodalL2Norm
    variable = total_res_disp_y
    outputs = console
  []

  [scaled_res_temp]
    type = ParsedPostprocessor
    pp_names = 'total_res_temp'
    expression = 'total_res_temp / ${temp_scaling}'
  []
  [scaled_res_disp_x]
    type = ParsedPostprocessor
    pp_names = 'total_res_disp_x'
    expression = 'total_res_disp_x / ${disp_x_scaling}'
  []
  [total_num_nonlinear_its]
    type = NumNonlinearIterations
  []
[]

[Outputs]
  csv = true
  time_step_interval = 9
[]

[Debug]
  show_var_residual_norms = true
[]
