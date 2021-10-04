[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [cube]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    elem_type = HEX8
  []
  [center_node]
    type = BoundingBoxNodeSetGenerator
    input = cube
    new_boundary = 'center_point'
    top_right = '0.51 0.51 0'
    bottom_left = '0.49 0.49 0'
  []
  [back_edge_y]
    type = BoundingBoxNodeSetGenerator
    input = center_node
    new_boundary = 'back_edge_y'
    bottom_left = '0.9 0.5 0'
    top_right = '1.1 0.5 0'
  []
  [back_edge_x]
    type = BoundingBoxNodeSetGenerator
    input = back_edge_y
    new_boundary = back_edge_x
    bottom_left = '0.5 0.9 0'
    top_right =   '0.5 1.0 0'
  []
[]

[AuxVariables]
  [temperature]
    initial_condition = 300
  []
  [pk2]
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
  [resolved_shear_stress_3]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_4]
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
  [forest_dislocations_3]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_4]
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
  [slip_resistance_0]
   order = CONSTANT
   family = MONOMIAL
  []
  [slip_resistance_3]
   order = CONSTANT
   family = MONOMIAL
  []
[]

[Modules/TensorMechanics/Master/all]
  strain = FINITE
  add_variables = true
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
  [slip_resistance_0]
    type = MaterialStdVectorAux
    variable = slip_resistance_0
    property = slip_resistance
    index = 0
    execute_on = timestep_end
  []
  [slip_resistance_3]
    type = MaterialStdVectorAux
    variable = slip_resistance_3
    property = slip_resistance
    index = 3
    execute_on = timestep_end
  []
[]

[BCs]
  [fix_y]
    type = DirichletBC
    variable = disp_y
    preset = true
    boundary = 'center_point back_edge_y'
    value = 0
  []
  [fix_x]
    type = DirichletBC
    variable = disp_x
    boundary = 'center_point back_edge_x'
    value = 0
  []
  [fix_z]
    type = DirichletBC
    variable = disp_z
    boundary = 'back'
    value = 0
  []
  [tdisp]
    type = FunctionDirichletBC
    variable = disp_z
    boundary = front
    function = '0.001*t'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensorConstantRotationCP
    C_ijkl = '1.622e5 9.18e4 6.88e4 1.622e5 6.88e4 1.805e5 4.67e4 4.67e4 4.67e4' #alpha Ti, Alankar et al. Acta Materialia 59 (2011) 7003-7009
    fill_method = symmetric9
    euler_angle_1 =  68
    euler_angle_2 =  14
    euler_angle_3 =  -53
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
    unit_cell_dimension = '2.934e-7 2.934e-7 4.657e-7' #Ti, in mm, https://materialsproject.org/materials/mp-46/
    temperature = temperature
    initial_forest_dislocation_density = 15.0e4
    initial_substructure_density = 5.0e2
    slip_system_modes = 2
    number_slip_systems_per_mode = '3 12'
    lattice_friction_per_mode = '1 1.5'
    effective_shear_modulus_per_mode = '4.7e4 4.7e4' #Ti, in MPa, https://materialsproject.org/materials/mp-46/
    burgers_vector_per_mode = '2.934e-7 6.586e-7' #Ti, in mm, https://materialsproject.org/materials/mp-46/
    slip_generation_coefficient_per_mode = '1e5 2e7'
    normalized_slip_activiation_energy_per_mode = '4e-3 3e-2'
    slip_energy_proportionality_factor_per_mode = '330 100'
    substructure_rate_coefficient_per_mode = '400 100'
    applied_strain_rate = 0.001
    gamma_o = 1.0e-3
    Hall_Petch_like_constant_per_mode = '0 0' #minimize impact
    grain_size = 20.0e-3 #20 microns
  []
[]

[Postprocessors]
  [pk2]
    type = ElementAverageValue
    variable = pk2
  []
  [e_zz]
    type = ElementAverageValue
    variable = e_zz
  []
  [tau_0]
    type = ElementAverageValue
    variable = resolved_shear_stress_0
  []
  [tau_3]
    type = ElementAverageValue
    variable = resolved_shear_stress_3
  []
  [tau_4]
    type = ElementAverageValue
    variable = resolved_shear_stress_4
  []
  [tau_8]
    type = ElementAverageValue
    variable = resolved_shear_stress_8
  []
  [tau_9]
    type = ElementAverageValue
    variable = resolved_shear_stress_9
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
  [forest_dislocation_3]
    type = ElementAverageValue
    variable = forest_dislocations_3
  []
  [forest_dislocation_4]
    type = ElementAverageValue
    variable = forest_dislocations_4
  []
  [forest_dislocation_8]
    type = ElementAverageValue
    variable = forest_dislocations_8
  []
  [forest_dislocation_9]
    type = ElementAverageValue
    variable = forest_dislocations_9
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

  [slip_resistance_0]
    type = ElementAverageValue
    variable = slip_resistance_0
  []
  [slip_resistance_3]
    type = ElementAverageValue
    variable = slip_resistance_3
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

  dt = 0.015
  dtmin = 1.0e-4
  dtmax = 0.1
  end_time = 0.15
[]

[Outputs]
  csv = true
[]
