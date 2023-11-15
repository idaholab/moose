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
  [e_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [pk2_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [fp_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [pin_point_density_0]
    order = CONSTANT
    family = MONOMIAL
  []
  [pin_point_density_1]
    order = CONSTANT
    family = MONOMIAL
  []
  [pin_point_density_2]
    order = CONSTANT
    family = MONOMIAL
  []
  [pin_point_density_3]
    order = CONSTANT
    family = MONOMIAL
  []
  [const_slip_incr_0]
    order = CONSTANT
    family = MONOMIAL
  []
  [const_slip_incr_1]
    order = CONSTANT
    family = MONOMIAL
  []
  [const_slip_incr_2]
    order = CONSTANT
    family = MONOMIAL
  []
  [const_slip_incr_3]
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
  [e_zz]
    type = RankTwoAux
    variable = e_zz
    rank_two_tensor = total_lagrangian_strain
    index_i = 2
    index_j = 2
    execute_on = timestep_end
  []
  [pk2_zz]
    type = RankTwoAux
    variable = pk2_zz
    rank_two_tensor = second_piola_kirchhoff_stress
    index_j = 2
    index_i = 2
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
  [pin_point_density_0]
    type = MaterialStdVectorAux
    variable = pin_point_density_0
    property = pinning_point_density
    index = 0
    execute_on = timestep_end
  []
  [pin_point_density_1]
    type = MaterialStdVectorAux
    variable = pin_point_density_1
    property = pinning_point_density
    index = 1
    execute_on = timestep_end
  []
  [pin_point_density_2]
    type = MaterialStdVectorAux
    variable = pin_point_density_2
    property = pinning_point_density
    index = 2
    execute_on = timestep_end
  []
  [pin_point_density_3]
    type = MaterialStdVectorAux
    variable = pin_point_density_3
    property = pinning_point_density
    index = 3
    execute_on = timestep_end
  []
  [const_slip_incr_0]
    type = MaterialStdVectorAux
    variable = const_slip_incr_0
    property = coplanar_constitutive_slip_increment
    index = 0
    execute_on = timestep_end
  []
  [const_slip_incr_1]
    type = MaterialStdVectorAux
    variable = const_slip_incr_1
    property = coplanar_constitutive_slip_increment
    index = 1
    execute_on = timestep_end
  []
  [const_slip_incr_2]
    type = MaterialStdVectorAux
    variable = const_slip_incr_2
    property = coplanar_constitutive_slip_increment
    index = 2
    execute_on = timestep_end
  []
  [const_slip_incr_3]
    type = MaterialStdVectorAux
    variable = const_slip_incr_3
    property = coplanar_constitutive_slip_increment
    index = 3
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
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = 'if(t<=0.2, 3.0e-3*t,
                if(t<=0.5, (6.0e-4 + 5.0e-4*(t-0.2)),
                if(t<=0.8, (7.5e-4 + 5.0e-5*(t-0.5)), 7.65e-4 + 1.0e-5*(t-0.8))))'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensorCP
    C_ijkl = '1.98e5 1.25e5 1.25e5 1.98e5 1.25e5 1.98e5 1.22e5 1.22e5 1.22e5'
    fill_method = symmetric9
    euler_angle_1 = 59
    euler_angle_2 = 37
    euler_angle_3 = 27
  []
  [stress]
    type = ComputeMultipleCrystalPlasticityStress
    crystal_plasticity_models = 'trial_xtalpl'
    tan_mod_type = exact
    line_search_method = CUT_HALF
    use_line_search = true
    maximum_substep_iteration = 5
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
[]

[Postprocessors]
  [e_zz]
    type = ElementAverageValue
    variable = e_zz
  []
  [pk2_zz]
    type = ElementAverageValue
    variable = pk2_zz
  []
  [fp_zz]
    type = ElementAverageValue
    variable = fp_zz
  []
  [pin_point_density_0]
    type = ElementAverageValue
    variable = pin_point_density_0
  []
  [pin_point_density_1]
    type = ElementAverageValue
    variable = pin_point_density_1
  []
  [pin_point_density_2]
    type = ElementAverageValue
    variable = pin_point_density_2
  []
  [pin_point_density_3]
    type = ElementAverageValue
    variable = pin_point_density_3
  []
  [const_slip_incr_0]
    type = ElementAverageValue
    variable = const_slip_incr_0
  []
  [const_slip_incr_1]
    type = ElementAverageValue
    variable = const_slip_incr_1
  []
  [const_slip_incr_2]
    type = ElementAverageValue
    variable = const_slip_incr_2
  []
  [const_slip_incr_3]
    type = ElementAverageValue
    variable = const_slip_incr_3
  []
[]

[Preconditioning]
  [smp]
    type = SMP
    full = true
  []
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'

  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
  petsc_options_value = ' asm      2              lu            gmres     200'
  nl_abs_tol = 1e-12
  nl_rel_tol = 1e-10
  nl_abs_step_tol = 1e-10
  nl_max_its = 15

  dt = 0.1
  dtmin = 1e-4
  num_steps = 20
[]

[Outputs]
  csv = true
  perf_graph = true
[]
