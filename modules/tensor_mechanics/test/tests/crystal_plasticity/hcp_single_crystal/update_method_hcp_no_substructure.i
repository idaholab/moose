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
  [resolved_shear_stress_3]
    order = CONSTANT
    family = MONOMIAL
  []
  [slip_resistance_3]
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
  [tau_3]
    type = MaterialStdVectorAux
    variable = resolved_shear_stress_3
    property = applied_shear_stress
    index = 3
    execute_on = timestep_end
  []
  [slip_resistance_3]
    type = MaterialStdVectorAux
    variable = slip_resistance_3
    property = slip_resistance
    index = 3
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
    function = '0.001*t'
  []
[]

[Materials]
  [elasticity_tensor]
    type = ComputeElasticityTensorCP
    C_ijkl = '1.622e5 9.18e4 6.88e4 1.622e5 6.88e4 1.805e5 4.67e4 4.67e4 4.67e4' #alpha Ti, Alankar et al. Acta Materialia 59 (2011) 7003-7009
    fill_method = symmetric9
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
    unit_cell_dimension = '2.934e-7 2.934e-7 4.657e-7'
    temperature = temperature
    initial_forest_dislocation_density = 15.0e5
    initial_substructure_density = 1.0 #artifically low for specific test
    slip_system_modes = 2
    number_slip_systems_per_mode = '3 12'
    lattice_friction_per_mode = '100 200'
    effective_shear_modulus_per_mode = '5e4 5e4'
    burgers_vector_per_mode = '2.934e-7 6.586e-7' #Ti, in mm, https://materialsproject.org/materials/mp-46/
    slip_generation_coefficient_per_mode = '1e5 2e7'
    normalized_slip_activiation_energy_per_mode = '4e-3 3e-2'
    slip_energy_proportionality_factor_per_mode = '300 100'
    substructure_rate_coefficient_per_mode = '-355 -0.4' #artifical, non-physical values for testing purposes
    applied_strain_rate = 0.001
    gamma_o = 1.0e-3
    Hall_Petch_like_constant_per_mode = '1 1'
    grain_size = 20.0e-3 #20 microns
  []
[]

[Postprocessors]
  [pk2]
    type = ElementAverageValue
    variable = pk2
  []
  [tau_3]
    type = ElementAverageValue
    variable = resolved_shear_stress_3
  []
  [slip_resistance_3]
    type = ElementAverageValue
    variable = slip_resistance_3
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

  dt = 0.5
  dtmin = 1.0e-2
  dtmax = 10.0
  end_time = 2.5
[]

[Outputs]
  csv = true
[]
