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
    initial_condition = 300
  []
  [pk2]
    order = CONSTANT
    family = MONOMIAL
  []
  [fp_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [e_zz]
    order = CONSTANT
    family = MONOMIAL
  []
  [resolved_shear_stress_0]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_1]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_2]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_3]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_4]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_5]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_6]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_7]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_8]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_9]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_10]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_11]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_12]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_13]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_14]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_0]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_1]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_2]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_3]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_4]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_5]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_6]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_7]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_8]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_9]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_10]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_11]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_12]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_13]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_14]
   order = CONSTANT
   family = MONOMIAL
  []
  [substructure_density]
   order = CONSTANT
   family = MONOMIAL
  []
[]

[Modules/TensorMechanics/Master/all]
  strain = FINITE
  add_variables = true
  generate_output = stress_zz
[]

[AuxKernels]
  [pk2]
    type = RankTwoAux
    variable = pk2
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
  [e_zz]
    type = RankTwoAux
    variable = e_zz
    rank_two_tensor = total_lagrangian_strain
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  []
  [tau_0]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_0
    property = applied_shear_stress
    index = 0
    execute_on = timestep_end
  []
  [tau_1]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_1
    property = applied_shear_stress
    index = 1
    execute_on = timestep_end
  []
  [tau_2]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_2
    property = applied_shear_stress
    index = 2
    execute_on = timestep_end
  []
  [tau_3]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_3
    property = applied_shear_stress
    index = 3
    execute_on = timestep_end
  []
  [tau_4]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_4
    property = applied_shear_stress
    index = 4
    execute_on = timestep_end
  []
  [tau_5]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_5
    property = applied_shear_stress
    index = 5
    execute_on = timestep_end
  []
  [tau_6]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_6
    property = applied_shear_stress
    index = 6
    execute_on = timestep_end
  []
  [tau_7]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_7
    property = applied_shear_stress
    index = 7
    execute_on = timestep_end
  []
  [tau_8]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_8
    property = applied_shear_stress
    index = 8
    execute_on = timestep_end
  []
  [tau_9]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_9
    property = applied_shear_stress
    index = 9
    execute_on = timestep_end
  []
  [tau_10]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_10
    property = applied_shear_stress
    index = 10
    execute_on = timestep_end
  []
  [tau_11]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_11
    property = applied_shear_stress
    index = 11
    execute_on = timestep_end
  []
  [tau_12]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_12
    property = applied_shear_stress
    index = 12
    execute_on = timestep_end
  []
  [tau_13]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_13
    property = applied_shear_stress
    index = 13
    execute_on = timestep_end
  []
  [tau_14]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_14
    property = applied_shear_stress
    index = 14
    execute_on = timestep_end
  []

  [forest_dislocations_0]
    type = MaterialStdVectorAux
    variable = forest_dislocations_0
    property = forest_dislocation_density
    index = 0
    execute_on = timestep_end
  []
  [forest_dislocations_1]
    type = MaterialStdVectorAux
    variable = forest_dislocations_1
    property = forest_dislocation_density
    index = 1
    execute_on = timestep_end
  []
  [forest_dislocations_2]
    type = MaterialStdVectorAux
    variable = forest_dislocations_2
    property = forest_dislocation_density
    index = 2
    execute_on = timestep_end
  []
  [forest_dislocations_3]
    type = MaterialStdVectorAux
    variable = forest_dislocations_3
    property = forest_dislocation_density
    index = 3
    execute_on = timestep_end
  []
  [forest_dislocations_4]
    type = MaterialStdVectorAux
    variable = forest_dislocations_4
    property = forest_dislocation_density
    index = 4
    execute_on = timestep_end
  []
  [forest_dislocations_5]
    type = MaterialStdVectorAux
    variable = forest_dislocations_5
    property = forest_dislocation_density
    index = 5
    execute_on = timestep_end
  []
  [forest_dislocations_6]
    type = MaterialStdVectorAux
    variable = forest_dislocations_6
    property = forest_dislocation_density
    index = 6
    execute_on = timestep_end
  []
  [forest_dislocations_7]
    type = MaterialStdVectorAux
    variable = forest_dislocations_7
    property = forest_dislocation_density
    index = 7
    execute_on = timestep_end
  []
  [forest_dislocations_8]
    type = MaterialStdVectorAux
    variable = forest_dislocations_8
    property = forest_dislocation_density
    index = 8
    execute_on = timestep_end
  []
  [forest_dislocations_9]
    type = MaterialStdVectorAux
    variable = forest_dislocations_9
    property = forest_dislocation_density
    index = 9
    execute_on = timestep_end
  []
  [forest_dislocations_10]
    type = MaterialStdVectorAux
    variable = forest_dislocations_10
    property = forest_dislocation_density
    index = 10
    execute_on = timestep_end
  []
  [forest_dislocations_11]
    type = MaterialStdVectorAux
    variable = forest_dislocations_11
    property = forest_dislocation_density
    index = 11
    execute_on = timestep_end
  []
  [forest_dislocations_12]
    type = MaterialStdVectorAux
    variable = forest_dislocations_12
    property = forest_dislocation_density
    index = 12
    execute_on = timestep_end
  []
  [forest_dislocations_13]
    type = MaterialStdVectorAux
    variable = forest_dislocations_13
    property = forest_dislocation_density
    index = 13
    execute_on = timestep_end
  []
  [forest_dislocations_14]
    type = MaterialStdVectorAux
    variable = forest_dislocations_14
    property = forest_dislocation_density
    index = 14
    execute_on = timestep_end
  []
  [substructure_density]
    type = MaterialRealAux
    variable = substructure_density
    property = total_substructure_density
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
    function = '0.01*t'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensorConstantRotationCP
    # C_ijkl = '1.622e5 9.18e4 6.88e4 1.622e5 6.88e4 1.805e5 4.67e4 4.67e4 4.67e4' #alpha Ti, Alankar et al. Acta Materialia 59 (2011) 7003-7009
    C_ijkl = '1.44e5 6.5e4 6.7e4 1.44e5 6.7e4 1.62e5 2.6e4 2.6e4 4.0e4' #Zr, in MPa, materialsproject.org, doi:10.17188/1189385
    fill_method = symmetric9
    # euler_angle_1 = 120.0
    # euler_angle_2 = 125.264
    # euler_angle_3 =  45.0
  []
  [stress]
    type = ComputeMultipleCrystalPlasticityStress
    crystal_plasticity_models = 'trial_xtalpl'
    tan_mod_type = exact
  []
  [trial_xtalpl]
    type = CrystalPlasticityHCPDislocationSlipBeyerleinUpdate
    number_slip_systems = 15
    slip_sys_file_name = hcp_aprismatic_capyramidal_slip_sys.txt
    unit_cell_dimension = '3.239e-7 3.239e-7 5.172e-7' #Zr, in mm, materialsproject.org, doi:10.17188/1189385
    temperature = temperature
    initial_forest_dislocation_density = 1.0e6 #estimated from Figure 2, Capolungo et al 2009
    initial_substructure_density = 1.0e4 #assumed, from well annealed crystal
    slip_system_modes = 2
    number_slip_systems_per_mode = '3 12'
    lattice_friction_per_mode = '15.26 161.2'
    effective_shear_modulus_per_mode = '3.346e4 3.346e4'
    burgers_vector_per_mode = '3.231e-7 6.077e-7'
    slip_generation_coefficient_per_mode = '1.25e5 2.25e7'
    normalized_slip_activiation_energy_per_mode = '3.73e-3 3.2e-2'
    slip_energy_proportionality_factor_per_mode = '330 100'
    substructure_rate_coefficient_per_mode = '355 0.4'
    applied_strain_rate = 0.001
    Hall_Petch_like_constant_per_mode = '100.0 170.0'
    grain_size = 20.0e3 #20 microns, Beyerlein and Tome
  []
[]

[Postprocessors]
  [stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  []
  [pk2]
    type = ElementAverageValue
    variable = pk2
  []
  [fp_zz]
    type = ElementAverageValue
    variable = fp_zz
  []
  [e_zz]
    type = ElementAverageValue
    variable = e_zz
  []
  [tau_0]
    type = ElementAverageValue
    variable = resolved_shear_stress_0
  []
  [tau_1]
    type = ElementAverageValue
    variable = resolved_shear_stress_1
  []
  [tau_2]
    type = ElementAverageValue
    variable = resolved_shear_stress_2
  []
  [tau_3]
    type = ElementAverageValue
    variable = resolved_shear_stress_3
  []
  [tau_4]
    type = ElementAverageValue
    variable = resolved_shear_stress_4
  []
  [tau_5]
    type = ElementAverageValue
    variable = resolved_shear_stress_5
  []
  [tau_6]
    type = ElementAverageValue
    variable = resolved_shear_stress_6
  []
  [tau_7]
    type = ElementAverageValue
    variable = resolved_shear_stress_7
  []
  [tau_8]
    type = ElementAverageValue
    variable = resolved_shear_stress_8
  []
  [tau_9]
    type = ElementAverageValue
    variable = resolved_shear_stress_9
  []
  [tau_10]
    type = ElementAverageValue
    variable = resolved_shear_stress_10
  []
  [tau_11]
    type = ElementAverageValue
    variable = resolved_shear_stress_11
  []
  [tau_12]
    type = ElementAverageValue
    variable = resolved_shear_stress_12
  []
  [tau_13]
    type = ElementAverageValue
    variable = resolved_shear_stress_13
  []
  [tau_14]
    type = ElementAverageValue
    variable = resolved_shear_stress_14
  []

  [forest_dislocation_0]
    type = ElementAverageValue
    variable = forest_dislocations_0
  []
  [forest_dislocation_1]
    type = ElementAverageValue
    variable = forest_dislocations_1
  []
  [forest_dislocation_2]
    type = ElementAverageValue
    variable = forest_dislocations_2
  []
  [forest_dislocation_3]
    type = ElementAverageValue
    variable = forest_dislocations_3
  []
  [forest_dislocation_4]
    type = ElementAverageValue
    variable = forest_dislocations_4
  []
  [forest_dislocation_5]
    type = ElementAverageValue
    variable = forest_dislocations_5
  []
  [forest_dislocation_6]
    type = ElementAverageValue
    variable = forest_dislocations_6
  []
  [forest_dislocation_7]
    type = ElementAverageValue
    variable = forest_dislocations_7
  []
  [forest_dislocation_8]
    type = ElementAverageValue
    variable = forest_dislocations_8
  []
  [forest_dislocation_9]
    type = ElementAverageValue
    variable = forest_dislocations_9
  []
  [forest_dislocation_10]
    type = ElementAverageValue
    variable = forest_dislocations_10
  []
  [forest_dislocation_11]
    type = ElementAverageValue
    variable = forest_dislocations_11
  []
  [forest_dislocation_12]
    type = ElementAverageValue
    variable = forest_dislocations_12
  []
  [forest_dislocation_13]
    type = ElementAverageValue
    variable = forest_dislocations_13
  []
  [forest_dislocation_14]
    type = ElementAverageValue
    variable = forest_dislocations_14
  []
  [substructure_density]
    type = ElementAverageValue
    variable = substructure_density
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
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
  petsc_options_value = ' asm      2              lu            gmres     200'
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
  nl_abs_step_tol = 1e-10

  dt = 0.05
  dtmin = 0.01
  dtmax = 10.0
  num_steps = 10
[]

[Outputs]
  csv = true
[]
