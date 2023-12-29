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
  [fp_zz]
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
  [resolved_shear_stress_12]
   order = CONSTANT
   family = MONOMIAL
  []
  [resolved_shear_stress_13]
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
  [forest_dislocations_12]
   order = CONSTANT
   family = MONOMIAL
  []
  [forest_dislocations_13]
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
  [slip_resistance_1]
   order = CONSTANT
   family = MONOMIAL
  []
  [slip_resistance_2]
   order = CONSTANT
   family = MONOMIAL
  []
  [slip_resistance_12]
   order = CONSTANT
   family = MONOMIAL
  []
  [slip_resistance_13]
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
  [fp_zz]
    type = RankTwoAux
    variable = fp_zz
    rank_two_tensor = plastic_deformation_gradient
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
  [slip_resistance_1]
    type = MaterialStdVectorAux
    variable = slip_resistance_1
    property = slip_resistance
    index = 1
    execute_on = timestep_end
  []
  [slip_resistance_2]
    type = MaterialStdVectorAux
    variable = slip_resistance_2
    property = slip_resistance
    index = 2
    execute_on = timestep_end
  []
  [slip_resistance_12]
    type = MaterialStdVectorAux
    variable = slip_resistance_12
    property = slip_resistance
    index = 12
    execute_on = timestep_end
  []
  [slip_resistance_13]
    type = MaterialStdVectorAux
    variable = slip_resistance_13
    property = slip_resistance
    index = 13
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
    type = ComputeElasticityTensorCP
    C_ijkl = '1.622e5 9.18e4 6.88e4 1.622e5 6.88e4 1.805e5 4.67e4 4.67e4 4.67e4' #alpha Ti, Alankar et al. Acta Materialia 59 (2011) 7003-7009
    fill_method = symmetric9
    euler_angle_1 = 164.5
    euler_angle_2 =  90.0
    euler_angle_3 =  15.3
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
    initial_forest_dislocation_density = 15.0e5
    initial_substructure_density = 1.0e3
    slip_system_modes = 2
    number_slip_systems_per_mode = '3 12'
    lattice_friction_per_mode = '0.5 5'
    effective_shear_modulus_per_mode = '4.7e4 4.7e4' #Ti, in MPa, https://materialsproject.org/materials/mp-46/
    burgers_vector_per_mode = '2.934e-7 6.586e-7' #Ti, in mm, https://materialsproject.org/materials/mp-46/
    slip_generation_coefficient_per_mode = '1e5 2e7'
    normalized_slip_activiation_energy_per_mode = '4e-3 3e-2'
    slip_energy_proportionality_factor_per_mode = '330 100'
    substructure_rate_coefficient_per_mode = '400 100'
    applied_strain_rate = 0.001
    gamma_o = 1.0e-3
    Hall_Petch_like_constant_per_mode = '2e-3 2e-3' #minimize impact
    grain_size = 20.0e-3 #20 microns
  []
[]

[Postprocessors]
  [pk2]
    type = ElementAverageValue
    variable = pk2
  []
  [fp_zz]
    type = ElementAverageValue
    variable = fp_zz
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
  [tau_12]
    type = ElementAverageValue
    variable = resolved_shear_stress_12
  []
  [tau_13]
    type = ElementAverageValue
    variable = resolved_shear_stress_13
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
  [forest_dislocation_12]
    type = ElementAverageValue
    variable = forest_dislocations_12
  []
  [forest_dislocation_13]
    type = ElementAverageValue
    variable = forest_dislocations_13
  []
  [substructure_density]
    type = ElementAverageValue
    variable = substructure_density
  []

  [slip_resistance_0]
    type = ElementAverageValue
    variable = slip_resistance_0
  []
  [slip_resistance_1]
    type = ElementAverageValue
    variable = slip_resistance_1
  []
  [slip_resistance_2]
    type = ElementAverageValue
    variable = slip_resistance_2
  []
  [slip_resistance_12]
    type = ElementAverageValue
    variable = slip_resistance_12
  []
  [slip_resistance_13]
    type = ElementAverageValue
    variable = slip_resistance_13
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
  nl_max_its = 20
  l_max_its = 50

  dt = 0.005
  dtmin = 1.0e-4
  dtmax = 0.1
  end_time = 0.09
[]

[Outputs]
  csv = true
[]
