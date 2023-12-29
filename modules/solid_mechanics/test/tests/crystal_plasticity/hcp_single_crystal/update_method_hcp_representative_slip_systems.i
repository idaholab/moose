[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
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
    function = '0.1*t'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensorCP
    C_ijkl = '1.622e5 9.18e4 6.88e4 1.622e5 6.88e4 1.805e5 4.67e4 4.67e4 4.67e4' #alpha Ti, Alankar et al. Acta Materialia 59 (2011) 7003-7009
    fill_method = symmetric9
    euler_angle_1 = 45
    euler_angle_2 = 60
    euler_angle_3 = 30
  []
  [stress]
    type = ComputeMultipleCrystalPlasticityStress
    crystal_plasticity_models = 'trial_xtalpl'
    tan_mod_type = exact
  []
  [trial_xtalpl]
    type = CrystalPlasticityHCPDislocationSlipBeyerleinUpdate
    number_slip_systems = 5
    slip_sys_file_name = select_input_slip_sys_hcp.txt
    unit_cell_dimension = '2.934e-7 2.934e-7 4.657e-7' #Ti, in mm, https://materialsproject.org/materials/mp-46/
    temperature = temperature
    initial_forest_dislocation_density = 15.0e5
    initial_substructure_density = 1.0e3
    slip_system_modes = 4
    number_slip_systems_per_mode = '1 1 2 1'
    lattice_friction_per_mode = '10 10 15 30'
    effective_shear_modulus_per_mode = '47e3 47e3 47e3 47e3'
    burgers_vector_per_mode = '2.934e-7 2.934e-7 2.934e-7 6.586e-7' #Ti, in mm, https://materialsproject.org/materials/mp-46/
    slip_generation_coefficient_per_mode = '2e7 1e5 2e7 2e7'
    normalized_slip_activiation_energy_per_mode = '3e-2 4e-3 3e-2 3e-2'
    slip_energy_proportionality_factor_per_mode = '100 330 100 100'
    substructure_rate_coefficient_per_mode = '100 400 1 1'
    applied_strain_rate = 0.001
    gamma_o = 1.0e-3
    strain_rate_sensitivity_exponent = 0.05
    Hall_Petch_like_constant_per_mode = '10 10 10 10'
    grain_size = 20.0e-3 #20 microns,
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
  dtmax = 0.1
  end_time = 0.4
[]

[Outputs]
  csv = true
[]
