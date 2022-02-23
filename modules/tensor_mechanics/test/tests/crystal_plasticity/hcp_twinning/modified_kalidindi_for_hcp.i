[GlobalParams]
  displacements = 'disp_x disp_y disp_z'
[]

[Mesh]
  [single_xtal]
    type = GeneratedMeshGenerator
    dim = 3
    nx = 2
    ny = 2
    nz = 2
    elem_type = HEX8
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
  [fp_xx]
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
  [total_twin_volume_fraction]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_volume_fraction_0]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_1]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_2]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_3]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_4]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_volume_fraction_5]
   order = CONSTANT
   family = MONOMIAL
  []
  [twin_increment_0]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_increment_1]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_increment_2]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_increment_3]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_increment_4]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_increment_5]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_resistance_0]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_resistance_1]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_resistance_2]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_resistance_3]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_resistance_4]
    order = CONSTANT
    family = MONOMIAL
  []
  [twin_resistance_5]
    order = CONSTANT
    family = MONOMIAL
  []
  [resolved_twin_stress_0]
    order = CONSTANT
    family = MONOMIAL
  []
  [resolved_twin_stress_1]
    order = CONSTANT
    family = MONOMIAL
  []
  [resolved_twin_stress_2]
    order = CONSTANT
    family = MONOMIAL
  []
  [resolved_twin_stress_3]
    order = CONSTANT
    family = MONOMIAL
  []
  [resolved_twin_stress_4]
    order = CONSTANT
    family = MONOMIAL
  []
  [resolved_twin_stress_5]
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
  [fp_xx]
    type = RankTwoAux
    variable = fp_xx
    rank_two_tensor = plastic_deformation_gradient
    index_j = 0
    index_i = 0
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
  [total_twin_volume_fraction]
    type = MaterialRealAux
    variable = total_twin_volume_fraction
    property = twin_total_volume_fraction_twins
    execute_on = timestep_end
  []
  [twin_volume_fraction_0]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_0
   property = twin_twin_system_volume_fraction
   index = 0
   execute_on = timestep_end
  []
  [twin_volume_fraction_1]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_1
   property = twin_twin_system_volume_fraction
   index = 1
   execute_on = timestep_end
  []
  [twin_volume_fraction_2]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_2
   property = twin_twin_system_volume_fraction
   index = 2
   execute_on = timestep_end
  []
  [twin_volume_fraction_3]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_3
   property = twin_twin_system_volume_fraction
   index = 3
   execute_on = timestep_end
  []
  [twin_volume_fraction_4]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_4
   property = twin_twin_system_volume_fraction
   index = 4
   execute_on = timestep_end
  []
  [twin_volume_fraction_5]
   type = MaterialStdVectorAux
   variable = twin_volume_fraction_5
   property = twin_twin_system_volume_fraction
   index = 5
   execute_on = timestep_end
  []
  [twin_resistance_0]
    type = MaterialStdVectorAux
    variable = twin_resistance_0
    property = twin_slip_resistance
    index = 0
    execute_on = timestep_end
  []
  [twin_resistance_1]
    type = MaterialStdVectorAux
    variable = twin_resistance_1
    property = twin_slip_resistance
    index = 1
    execute_on = timestep_end
  []
  [twin_resistance_2]
    type = MaterialStdVectorAux
    variable = twin_resistance_2
    property = twin_slip_resistance
    index = 2
    execute_on = timestep_end
  []
  [twin_resistance_3]
    type = MaterialStdVectorAux
    variable = twin_resistance_3
    property = twin_slip_resistance
    index = 3
    execute_on = timestep_end
  []
  [twin_resistance_4]
    type = MaterialStdVectorAux
    variable = twin_resistance_4
    property = twin_slip_resistance
    index = 4
    execute_on = timestep_end
  []
  [twin_resistance_5]
    type = MaterialStdVectorAux
    variable = twin_resistance_5
    property = twin_slip_resistance
    index = 5
    execute_on = timestep_end
  []
  [twin_increment_0]
    type = MaterialStdVectorAux
    variable = twin_increment_0
    property = twin_slip_increment
    index = 0
    execute_on = timestep_end
  []
  [twin_increment_1]
    type = MaterialStdVectorAux
    variable = twin_increment_1
    property = twin_slip_increment
    index = 1
    execute_on = timestep_end
  []
  [twin_increment_2]
    type = MaterialStdVectorAux
    variable = twin_increment_2
    property = twin_slip_increment
    index = 2
    execute_on = timestep_end
  []
  [twin_increment_3]
    type = MaterialStdVectorAux
    variable = twin_increment_3
    property = twin_slip_increment
    index = 3
    execute_on = timestep_end
  []
  [twin_increment_4]
    type = MaterialStdVectorAux
    variable = twin_increment_4
    property = twin_slip_increment
    index = 4
    execute_on = timestep_end
  []
  [twin_increment_5]
    type = MaterialStdVectorAux
    variable = twin_increment_5
    property = twin_slip_increment
    index = 5
    execute_on = timestep_end
  []
  [twin_tau_0]
    type = MaterialStdVectorAux
    variable = resolved_twin_stress_0
    property = twin_applied_shear_stress
    index = 0
    execute_on = timestep_end
  []
  [twin_tau_1]
    type = MaterialStdVectorAux
    variable = resolved_twin_stress_1
    property = twin_applied_shear_stress
    index = 1
    execute_on = timestep_end
  []
  [twin_tau_2]
    type = MaterialStdVectorAux
    variable = resolved_twin_stress_2
    property = twin_applied_shear_stress
    index = 2
    execute_on = timestep_end
  []
  [twin_tau_3]
    type = MaterialStdVectorAux
    variable = resolved_twin_stress_3
    property = twin_applied_shear_stress
    index = 3
    execute_on = timestep_end
  []
  [twin_tau_4]
    type = MaterialStdVectorAux
    variable = resolved_twin_stress_4
    property = twin_applied_shear_stress
    index = 4
    execute_on = timestep_end
  []
  [twin_tau_5]
    type = MaterialStdVectorAux
    variable = resolved_twin_stress_5
    property = twin_applied_shear_stress
    index = 5
    execute_on = timestep_end
  []
[]

[BCs]
  [symmy]
    type = DirichletBC
    variable = disp_y
    preset = true
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
    C_ijkl = '1.622e5 9.18e4 6.88e4 1.622e5 6.88e4 1.805e5 4.67e4 4.67e4 4.67e4' #alpha Ti, Alankar et al. Acta Materialia 59 (2011) 7003-7009
    fill_method = symmetric9
  []
  [stress]
    type = ComputeMultipleCrystalPlasticityStress
    crystal_plasticity_models = 'twin_xtalpl'
    tan_mod_type = exact
  []
  [twin_xtalpl]
    type = CrystalPlasticityTwinningKalidindiUpdate
    base_name = twin
    crystal_lattice_type = HCP
    unit_cell_dimension = '2.934e-7 2.934e-7 4.657e-7' #Ti, in mm, https://materialsproject.org/materials/mp-46/
    number_slip_systems = 6
    slip_sys_file_name = 'hcp_tensile_twin_systems.txt'
    initial_twin_lattice_friction = 1140
    non_coplanar_coefficient_twin_hardening = 10000
    coplanar_coefficient_twin_hardening = 1000
    characteristic_twin_shear = 0.167
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
  [fp_xx]
    type = ElementAverageValue
    variable = fp_xx
  []
  [fp_zz]
    type = ElementAverageValue
    variable = fp_zz
  []
  [e_zz]
    type = ElementAverageValue
    variable = e_zz
  []
  [total_twin_volume_fraction]
    type = ElementAverageValue
    variable = total_twin_volume_fraction
  []
  [twin_volume_fraction_0]
    type = ElementAverageValue
    variable = twin_volume_fraction_0
  []
  [twin_volume_fraction_1]
    type = ElementAverageValue
    variable = twin_volume_fraction_1
  []
  [twin_volume_fraction_2]
    type = ElementAverageValue
    variable = twin_volume_fraction_2
  []
  [twin_volume_fraction_3]
    type = ElementAverageValue
    variable = twin_volume_fraction_3
  []
  [twin_volume_fraction_4]
    type = ElementAverageValue
    variable = twin_volume_fraction_4
  []
  [twin_volume_fraction_5]
    type = ElementAverageValue
    variable = twin_volume_fraction_5
  []
  [twin_resistance_0]
    type = ElementAverageValue
    variable = twin_resistance_0
  []
  [twin_resistance_1]
    type = ElementAverageValue
    variable = twin_resistance_1
  []
  [twin_resistance_2]
    type = ElementAverageValue
    variable = twin_resistance_2
  []
  [twin_resistance_3]
    type = ElementAverageValue
    variable = twin_resistance_3
  []
  [twin_resistance_4]
    type = ElementAverageValue
    variable = twin_resistance_4
  []
  [twin_resistance_5]
    type = ElementAverageValue
    variable = twin_resistance_5
  []
  [twin_increment_0]
    type = ElementAverageValue
    variable = twin_increment_0
  []
  [twin_increment_1]
    type = ElementAverageValue
    variable = twin_increment_1
  []
  [twin_increment_2]
    type = ElementAverageValue
    variable = twin_increment_2
  []
  [twin_increment_3]
    type = ElementAverageValue
    variable = twin_increment_3
  []
  [twin_increment_4]
    type = ElementAverageValue
    variable = twin_increment_4
  []
  [twin_increment_5]
    type = ElementAverageValue
    variable = twin_increment_5
  []
  [twin_tau_0]
    type = ElementAverageValue
    variable = resolved_twin_stress_0
  []
  [twin_tau_1]
    type = ElementAverageValue
    variable = resolved_twin_stress_1
  []
  [twin_tau_2]
    type = ElementAverageValue
    variable = resolved_twin_stress_2
  []
  [twin_tau_3]
    type = ElementAverageValue
    variable = resolved_twin_stress_3
  []
  [twin_tau_4]
    type = ElementAverageValue
    variable = resolved_twin_stress_4
  []
  [twin_tau_5]
    type = ElementAverageValue
    variable = resolved_twin_stress_5
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

  dt = 0.5
  dtmin = 1.0e-2
  dtmax = 10.0
  end_time = 2.5
[]

[Outputs]
  csv = true
[]
