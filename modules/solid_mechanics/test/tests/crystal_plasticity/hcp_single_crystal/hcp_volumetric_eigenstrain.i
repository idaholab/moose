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
  [ev_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [e_xtalpl_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [fv_xx]
    order = CONSTANT
    family = MONOMIAL
  []
  [fv_yy]
    order = CONSTANT
    family = MONOMIAL
  []
  [fv_zz]
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

[Physics/SolidMechanics/QuasiStatic/all]
  strain = FINITE
  incremental = true
  add_variables = true
  additional_generate_output = 'stress_zz stress_xx stress_yy vonmises_stress l2norm_strain'
  additional_material_output_order = FIRST
[]

[Functions]
  [temperature_ramp]
    type = ParsedFunction
    expression = 'if(t<=1500.0, 600.0 + t/6.0, 850.0)'
  []
[]

[AuxKernels]
  [temperature]
    type = FunctionAux
    variable = temperature
    function = 'temperature_ramp'
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
  [ev_zz]
    type = RankTwoAux
    variable = ev_zz
    rank_two_tensor = void_eigenstrain
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
  [fv_xx]
    type = RankTwoAux
    variable = fv_xx
    rank_two_tensor = volumetric_deformation_gradient
    index_j = 0
    index_i = 0
    execute_on = timestep_end
  []
  [fv_yy]
    type = RankTwoAux
    variable = fv_yy
    rank_two_tensor = volumetric_deformation_gradient
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  []
  [fv_zz]
    type = RankTwoAux
    variable = fv_zz
    rank_two_tensor = volumetric_deformation_gradient
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
    value = 0
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensorCP
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 0.754e5 0.754e5 0.754e5'
    fill_method = symmetric9
  []
  [stress]
    type = ComputeMultipleCrystalPlasticityStress
    crystal_plasticity_models = 'trial_xtalpl'
    eigenstrain_names = void_eigenstrain
    tan_mod_type = exact
    line_search_method = CUT_HALF
    use_line_search = true
    maximum_substep_iteration = 10
  []
  [trial_xtalpl]
    type = CrystalPlasticityHCPDislocationSlipBeyerleinUpdate
    number_slip_systems = 15
    slip_sys_file_name = hcp_aprismatic_capyramidal_slip_sys.txt
    unit_cell_dimension = '2.934e-7 2.934e-7 4.657e-7' #Ti, in mm, https://materialsproject.org/materials/mp-46/
    temperature = temperature
    initial_forest_dislocation_density = 15.0e3
    initial_substructure_density = 1.0e3
    slip_system_modes = 2
    number_slip_systems_per_mode = '3 12'
    lattice_friction_per_mode = '9 22' #Knezevic et al MSEA 654 (2013)
    effective_shear_modulus_per_mode = '4.7e2 4.7e2' #Ti, in MPa, https://materialsproject.org/materials/mp-46/
    burgers_vector_per_mode = '2.934e-7 6.586e-7' #Ti, in mm, https://materialsproject.org/materials/mp-46/
    slip_generation_coefficient_per_mode = '1.25e5 2.25e7' #from Beyerlein and Tome 2008 IJP
    normalized_slip_activiation_energy_per_mode = '3.73e-3 3.2e-2' #from Beyerlein and Tome 2008 IJP
    slip_energy_proportionality_factor_per_mode = '330 100' #from Beyerlein and Tome 2008 IJP
    substructure_rate_coefficient_per_mode = '355 0.4' #from Capolungo et al MSEA (2009)
    applied_strain_rate = 0.001
    gamma_o = 1.0e-3
    Hall_Petch_like_constant_per_mode = '0.2 0.2' #Estimated to match graph in Capolungo et al MSEA (2009), Figure 2
    grain_size = 20.0e-3 #20 microns, Beyerlein and Tome IJP (2008)
  []
  [void_eigenstrain]
    type = ComputeCrystalPlasticityVolumetricEigenstrain
    eigenstrain_name = void_eigenstrain
    deformation_gradient_name = volumetric_deformation_gradient
    mean_spherical_void_radius = void_radius
    spherical_void_number_density = void_density
  []
  [void_density]
    type = ParsedMaterial
    property_name = void_density
    coupled_variables = temperature
    expression = 'if(temperature<611.0, 0.0,
                    if(temperature<=835.0, 2.387e13 *(temperature - 611.0) / 1344.0, 0.0))' #1/mm^3, gives an eigenstrain of 1.0e-5 with radius=1.0e-6mm
    # outputs = exodus
  []
  [void_radius]
    type = GenericConstantMaterial
    prop_names = void_radius
    prop_values = '1.0e-6'  ##1 nm avg particle radius
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
  [ev_zz]
    type = ElementAverageValue
    variable = ev_zz
  []
  [e_xtalpl_zz]
    type = ElementAverageValue
    variable = e_xtalpl_zz
[]
  [fv_xx]
    type = ElementAverageValue
    variable = fv_xx
  []
  [fv_yy]
    type = ElementAverageValue
    variable = fv_yy
  []
  [fv_zz]
    type = ElementAverageValue
    variable = fv_zz
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
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
  petsc_options_value = ' asm      2              lu            gmres     200'
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
  nl_abs_step_tol = 1e-10

  dt = 10.0
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
