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
[]

[Modules/TensorMechanics/Master/all]
  strain = FINITE
  add_variables = true
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
    print_state_variable_convergence_error_messages = true
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
    print_state_variable_convergence_error_messages = true
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

  dt = 0.3
  dtmin = 1.0e-4
  dtmax = 0.1
  num_steps = 1
[]
