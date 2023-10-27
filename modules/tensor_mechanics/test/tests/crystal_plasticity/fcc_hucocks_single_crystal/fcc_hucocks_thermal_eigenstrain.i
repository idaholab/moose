[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  nx = 2
  ny = 2
  nz = 2
  elem_type = HEX8
[]

[AuxVariables]
  [temperature]
    order = FIRST
    family = LAGRANGE
  []
  [e_xtalpl_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [e_xtalpl_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [eth_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [e_xtalpl_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [fth_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [fth_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [fth_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [fp_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [fp_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [fp_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [f_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [f_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [f_zz]
    order = CONSTANT
    family = MONOMIAL
  []
[]

[Modules/TensorMechanics/Master/all]
  strain = FINITE
  incremental = true
  add_variables = true
  generate_output = stress_zz
[]

[AuxKernels]
  [temperature]
    type = FunctionAux
    variable = temperature
    function = '300+400*t' # temperature increases at a constant rate
    execute_on = timestep_begin
  []
  [e_xtalpl_xx]
    type = RankTwoAux
    variable = e_xtalpl_xx
    rank_two_tensor = total_lagrangian_strain
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  []
  [e_xtalpl_yy]
    type = RankTwoAux
    variable = e_xtalpl_yy
    rank_two_tensor = total_lagrangian_strain
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  []
  [eth_zz]
    type = RankTwoAux
    variable = eth_zz
    rank_two_tensor = thermal_eigenstrain
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
  [e_xtalpl_zz]
    type = RankTwoAux
    variable = e_xtalpl_zz
    rank_two_tensor = total_lagrangian_strain
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  []
  [fth_xx]
    type = RankTwoAux
    variable = fth_xx
    rank_two_tensor = thermal_deformation_gradient
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  []
  [fth_yy]
    type = RankTwoAux
    variable = fth_yy
    rank_two_tensor = thermal_deformation_gradient
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  []
  [fth_zz]
    type = RankTwoAux
    variable = fth_zz
    rank_two_tensor = thermal_deformation_gradient
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
  [fp_xx]
    type = RankTwoAux
    variable = fp_xx
    rank_two_tensor = plastic_deformation_gradient
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  []
  [fp_yy]
    type = RankTwoAux
    variable = fp_yy
    rank_two_tensor = plastic_deformation_gradient
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  []
  [fp_zz]
    type = RankTwoAux
    variable = fp_zz
    rank_two_tensor = plastic_deformation_gradient
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
  [f_xx]
    type = RankTwoAux
    variable = f_xx
    rank_two_tensor = deformation_gradient
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  []
  [f_yy]
    type = RankTwoAux
    variable = f_yy
    rank_two_tensor = deformation_gradient
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  []
  [f_zz]
    type = RankTwoAux
    variable = f_zz
    rank_two_tensor = deformation_gradient
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
[]

[BCs]
  [symmy]
    type = DirichletBC
    variable = disp_y
    boundary = bottom
    value = 0
  []
  [symmx]
    type = DirichletBC
    variable = disp_x
    boundary = left
    value = 0
  []
  [symmz]
    type = DirichletBC
    variable = disp_z
    boundary = back
    value = 0
  []
  [tdisp]
    type = DirichletBC
    variable = disp_z
    boundary = front
    value = 0.0 #'0.1*t'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensorCP
    C_ijkl = '1.98e5 1.25e5 1.25e5 1.98e5 1.25e5 1.98e5 1.22e5 1.22e5 1.22e5'
    fill_method = symmetric9
  []
  [stress]
    type = ComputeMultipleCrystalPlasticityStress
    crystal_plasticity_models = 'trial_xtalpl'
    eigenstrain_names = thermal_eigenstrain
    tan_mod_type = exact
    line_search_method = CUT_HALF
    use_line_search = true
    maximum_substep_iteration = 10
  []
  [trial_xtalpl]
    type = CrystalPlasticityFCCDislocationLinkHuCocksUpdate
    number_slip_systems = 12
    slip_sys_file_name = input_slip_sys.txt
    number_coplanar_groups = 4
    shear_modulus = 1.22e5 #in MPa, from Hu et al 2016 IJP
    burgers_vector = 2.5e-7 #in mm, from Hu et al 2016 IJP
    initial_pinning_point_density = 2.4e7 ## in 1/mm^2, from Hu et al 2016 IJP
    coefficient_self_plane_evolution = 5.0e8 ## in 1/mm^2, from Hu et al 2016 IJP
    coefficient_latent_plane_evolution = 2.5e9 ## in 1/mm^2, from Hu et al 2016 IJP
    forest_dislocation_hardening_coefficient = 0.35 #unitless, Madec et al 2002 via Hu et al 2016 IJP
    solute_hardening_coefficient = 0.00457 #unitless, Hu et al 2016 IJP
    solute_concentration = solute_conc
    precipitate_number_density = precipitate_conc
    mean_precipitate_radius = precipitate_radius
    precipitate_hardening_coefficient = 0.84 #unitless, Foreman and Makin 1966 via Hu et al 2016 IJP
    stol = 5.0e-3
    resistance_tol = 5.0e-3
    zero_tol = 1e-16
    # print_state_variable_convergence_error_messages = true
  []
  [concentrations]
    type = GenericConstantMaterial
    prop_names = 'solute_conc             precipitate_conc    precipitate_radius'
    prop_values = '2.818586455498711e+17  597211024.5155126     1.0e-3' # in 1/mm^3, backed out from given stress, except radius which is assumed
  []
  [thermal_eigenstrain]
    type = ComputeCrystalPlasticityThermalEigenstrain
    eigenstrain_name = thermal_eigenstrain
    deformation_gradient_name = thermal_deformation_gradient
    temperature = temperature
    thermal_expansion_coefficients = '0.5e-05 0.5e-05 0.5e-05' # thermal expansion coefficients along three directions
  []
[]

[Postprocessors]
  [stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  []
  [e_xtalpl_xx]
    type = ElementAverageValue
    variable = e_xtalpl_xx
  []
  [e_xtalpl_yy]
    type = ElementAverageValue
    variable = e_xtalpl_yy
  []
  [eth_zz]
    type = ElementAverageValue
    variable = eth_zz
  []
  [e_xtalpl_zz]
    type = ElementAverageValue
    variable = e_xtalpl_zz
  []
  [fth_xx]
    type = ElementAverageValue
    variable = fth_xx
  []
  [fth_yy]
    type = ElementAverageValue
    variable = fth_yy
  []
  [fth_zz]
    type = ElementAverageValue
    variable = fth_zz
  []
  [temperature]
    type = ElementAverageValue
    variable = temperature
  []
  [fp_xx]
    type = ElementAverageValue
    variable = fp_xx
  []
  [fp_yy]
    type = ElementAverageValue
    variable = fp_yy
  []
  [fp_zz]
    type = ElementAverageValue
    variable = fp_zz
  []
  [f_xx]
    type = ElementAverageValue
    variable = f_xx
  []
  [f_yy]
    type = ElementAverageValue
    variable = f_yy
  []
  [f_zz]
    type = ElementAverageValue
    variable = f_zz
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Debug]
  show_var_residual_norms = true
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
  petsc_options_value = ' asm      2              lu            gmres     200'
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
  nl_abs_step_tol = 1e-10

  dt = 0.1
  dtmin = 1e-4
  # end_time = 10
  num_steps = 10
[]

[Outputs]
  csv = true
  [console]
    type = Console
    # max_rows = 5
  []
[]
