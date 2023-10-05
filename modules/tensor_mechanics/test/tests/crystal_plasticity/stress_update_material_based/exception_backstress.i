[GlobalParams]
  displacements = 'ux uy uz'
[]

[Mesh]
  type = GeneratedMesh
  dim = 3
  elem_type = HEX8
[]

[AuxVariables]
  [./pk2]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./fp_zz]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./rotout]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./e_zz]
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
  [./backstress]
   order = CONSTANT
   family = MONOMIAL
  [../]
[]

[Modules/TensorMechanics/Master/all]
  strain = FINITE
  add_variables = true
  generate_output = stress_zz
[]

[AuxKernels]
  [./fp_zz]
    type = RankTwoAux
    variable = fp_zz
    rank_two_tensor = plastic_deformation_gradient
    index_j = 2
    index_i = 2
    execute_on = timestep_end
  [../]
  [./pk2]
   type = RankTwoAux
   variable = pk2
   rank_two_tensor = second_piola_kirchhoff_stress
   index_j = 2
   index_i = 2
   execute_on = timestep_end
  [../]
  [./e_zz]
    type = RankTwoAux
    variable = e_zz
    rank_two_tensor = total_lagrangian_strain
    index_j = 2
    index_i = 2
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
  [./backstress]
   type = MaterialStdVectorAux
   variable = backstress
   property = backstress
   index = 0
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
  [./symmz]
    type = DirichletBC
    variable = uz
    boundary = back
    value = 0
  [../]
  [./tdisp]
    type = FunctionDirichletBC
    variable = uz
    boundary = front
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
    rtol = 1e-6 # Constitutive stress residual relative tolerance
    maxiter_state_variable = 50 # Maximum number of iterations for stress update

    maximum_substep_iteration = 25 # Maximum number of substep iteration

    use_line_search = true
  [../]
  [./trial_xtalpl]
    type = CrystalPlasticityKalidindiBackstress
    crystal_lattice_type = FCC
    number_slip_systems = 12 
    slip_sys_file_name = input_slip_sys.txt

    slip_increment_tolerance = 0.1 # Maximum allowable slip in an increment
    stol = 0.1 # Constitutive internal state variable relative change tolerance
    resistance_tol = 0.1

    # https://www.sciencedirect.com/science/article/pii/S0921509317300898
    h = 1.0e4
    h_D = 1.0e2
  [../]
[]

[Postprocessors]
  [./stress_zz]
    type = ElementAverageValue
    variable = stress_zz
  [../]
  [./pk2]
   type = ElementAverageValue
   variable = pk2
  [../]
  [./fp_zz]
    type = ElementAverageValue
    variable = fp_zz
  [../]
  [./e_zz]
    type = ElementAverageValue
    variable = e_zz
  [../]
  [./gss]
    type = ElementAverageValue
    variable = gss
  [../]
  [./slip_increment]
   type = ElementAverageValue
   variable = slip_increment
  [../]
  [./backstress]
    type = ElementAverageValue
    variable = backstress
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
  num_steps = 100
  dtmin = 0.1e-6

  [./TimeStepper]
    type = IterationAdaptiveDT
    dt = 0.01 # Initial time step.  In this simulation it changes.
    optimal_iterations = 30 # Time step will adapt to maintain this number of nonlinear iterations
    iteration_window = 5
  [../]
[]

[Outputs]
  [my_exodus]
    file_base = ./ex_backstress_tensile/out_backstress_tensile
    interval = 10
    type = Nemesis
    additional_execute_on = 'FINAL'
  [../]
  [./csv]
    file_base = ./csv_backstress_tensile/out_backstress_tensile
    type = CSV
  [../]
[]