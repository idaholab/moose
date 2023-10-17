[GlobalParams]
  displacements = 'ux uy'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 10
  ny = 10
  xmax = 10
  ymax = 10
  elem_type = QUAD4
[]

[AuxVariables]
  [./pk2]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./fp_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./rotout]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./gss]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./slip_increment]
   order = CONSTANT
   family = MONOMIAL
  [../]
  [./backstress_0]
   order = CONSTANT
   family = MONOMIAL
  [../]
  [./backstress_1]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./backstress_2]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./backstress_3]
    order = CONSTANT
    family = MONOMIAL
  [../] 
  [./backstress_4]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./backstress_5]
    order = CONSTANT
    family = MONOMIAL
  [../]    
[]

[Modules/TensorMechanics/Master/all]
  strain = FINITE
  add_variables = true
  generate_output = stress_yy
[]

[AuxKernels]
  [./fp_yy]
    type = RankTwoAux
    variable = fp_yy
    rank_two_tensor = plastic_deformation_gradient
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  [../]
  [./pk2]
   type = RankTwoAux
   variable = pk2
   rank_two_tensor = second_piola_kirchhoff_stress
   index_j = 1
   index_i = 1
   execute_on = timestep_end
  [../]
  [./e_yy]
    type = RankTwoAux
    variable = e_yy
    rank_two_tensor = total_lagrangian_strain
    index_j = 1
    index_i = 1
    execute_on = timestep_end
  [../]
  [./gss]
    type = MaterialStdVectorAux
    variable = gss
    property = slip_resistance
    index = 0
    execute_on = timestep_end
  [../]
  [./slip_inc]
   type = MaterialStdVectorAux
   variable = slip_increment
   property = slip_increment
   index = 0
   execute_on = timestep_end
  [../]
  [./backstress_0]
   type = MaterialStdVectorAux
   variable = backstress_0
   property = backstress
   index = 0
   execute_on = timestep_end
  [../]
  [./backstress_1]
   type = MaterialStdVectorAux
   variable = backstress_1
   property = backstress
   index = 1
   execute_on = timestep_end
  [../]
  [./backstress_2]
    type = MaterialStdVectorAux
    variable = backstress_2
    property = backstress
    index = 2
    execute_on = timestep_end
  [../]
  [./backstress_3]
  type = MaterialStdVectorAux
  variable = backstress_3
  property = backstress
  index = 3
  execute_on = timestep_end
  [../]
  [./backstress_4]
    type = MaterialStdVectorAux
    variable = backstress_4
    property = backstress
    index = 4
    execute_on = timestep_end
  [../]
  [./backstress_5]
  type = MaterialStdVectorAux
  variable = backstress_5
  property = backstress
  index = 5
  execute_on = timestep_end
  [../]
[]

[BCs]
  [./symmy]
    type = DirichletBC
    variable = uy
    boundary = bottom
    value = 0
  [../]
  [./symmx]
    type = DirichletBC
    variable = ux
    boundary = left
    value = 0
  [../]
  [./tdisp]
    type = FunctionDirichletBC
    variable = uy
    boundary = top
    function = '0.1*t'
  [../]
[]

[Materials]
  [./elasticity_tensor]
    type = ComputeElasticityTensorCP
    C_ijkl = '1.684e5 1.214e5 1.214e5 1.684e5 1.214e5 1.684e5 0.754e5 0.754e5 0.754e5'
    fill_method = symmetric9
  [../]
  [./stress]
    type = ComputeMultipleCrystalPlasticityStress
    crystal_plasticity_models = 'trial_xtalpl'
    tan_mod_type = exact

    # rtol = 1e-6 # Constitutive stress residual relative tolerance
    # maxiter_state_variable = 50 # Maximum number of iterations for stress update
    # maximum_substep_iteration = 25 # Maximum number of substep iteration

    use_line_search = true
  [../]
  [./trial_xtalpl]
    type = CrystalPlasticityKalidindiBackstressUpdate # CrystalPlasticityKalidindiUpdate
    crystal_lattice_type = FCC
    number_slip_systems = 12 
    slip_sys_file_name = input_slip_sys.txt

    gss_initial = 30.8
    t_sat = 148
    # h = 180
    slip_increment_tolerance = 0.1 # Maximum allowable slip in an increment
    stol = 0.1 # Constitutive internal state variable relative change tolerance
    resistance_tol = 0.1

    # https://www.sciencedirect.com/science/article/pii/S0921509317300898
    c_bs = 1.0e4 # 1.0e4
    d_bs = 1.0e2 # 1.0e2
  [../]
[]

[Postprocessors]
  [./stress_yy]
    type = ElementAverageValue
    variable = stress_yy
  [../]
  [./pk2]
   type = ElementAverageValue
   variable = pk2
  [../]
  [./fp_yy]
    type = ElementAverageValue
    variable = fp_yy
  [../]
  [./e_yy]
    type = ElementAverageValue
    variable = e_yy
  [../]
  [./gss]
    type = ElementAverageValue
    variable = gss
  [../]
  [./slip_increment]
   type = ElementAverageValue
   variable = slip_increment
  [../]
  [./backstress_0]
    type = ElementAverageValue
    variable = backstress_0
  [../]
  [./backstress_1]
    type = ElementAverageValue
    variable = backstress_1
  [../]
  [./backstress_2]
    type = ElementAverageValue
    variable = backstress_2
  [../]
  [./backstress_3]
    type = ElementAverageValue
    variable = backstress_3
  [../]
  [./backstress_4]
    type = ElementAverageValue
    variable = backstress_4
  [../]
  [./backstress_5]
    type = ElementAverageValue
    variable = backstress_5
  [../]
  [./run_time]
    type = PerfGraphData
    section_name = "Root"
    data_type = total
  [../]
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'
  petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -ksp_type -ksp_gmres_restart'
  petsc_options_value = ' asm      2              lu            gmres     200'
  nl_abs_tol = 1e-10
  nl_rel_tol = 1e-10
  nl_abs_step_tol = 1e-10
  nl_max_its = 20 # Max number of nonlinear iterations

  start_time = 0.0
  # end_time = 1.0
  num_steps = 3
  dt = 0.025

  dtmin = 0.1e-6
  dtmax = 0.1
[]

[Outputs]
  file_base = 'exception_backstress_out'
  [./csv]
    type = CSV
  [../]
[]