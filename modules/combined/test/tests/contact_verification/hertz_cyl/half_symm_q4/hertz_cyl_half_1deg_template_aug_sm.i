[Mesh]
  file = hertz_cyl_half_1deg.e
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
[]

[Problem]
  type = AugmentedLagrangianContactProblem
  solution_variables = 'disp_x disp_y'
  reference_residual_variables = 'saved_x saved_y'
  maximum_lagrangian_update_iterations = 100
  penetration_tolerance = 1e-4
  tangential_increment_stick_tolerance = 1e-2
  tangential_friction_force_tolerance =  1e-2
[]

[Variables]
  [./disp_x]
    initial_condition = 0.0
  [../]
  [./disp_y]
    initial_condition = 0.0
  [../]
[]

[AuxVariables]
  [./contact_traction]
  [../]
  [./stress_xx]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_yy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./stress_xy]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./saved_x]
  [../]
  [./saved_y]
  [../]
  [./diag_saved_x]
  [../]
  [./diag_saved_y]
  [../]
  [./inc_slip_x]
  [../]
  [./inc_slip_y]
  [../]
  [./accum_slip_x]
  [../]
  [./accum_slip_y]
  [../]
  [./tang_force_x]
  [../]
  [./tang_force_y]
  [../]
[]

[Functions]
  [./disp_ramp_vert]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. -0.0020 -0.0020'
  [../]
  [./disp_ramp_horz]
    type = PiecewiseLinear
    x = '0. 1. 2.'
    y = '0. 0.0 0.001'
  [../]
[]

[SolidMechanics]
  [./solid]
    disp_x = disp_x
    disp_y = disp_y
    save_in_disp_y = saved_y
    save_in_disp_x = saved_x
    diag_save_in_disp_y = diag_saved_y
    diag_save_in_disp_x = diag_saved_x
  [../]
[]

[AuxKernels]
  [./stress_xx]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xx
    index = 0
  [../]
  [./stress_yy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_yy
    index = 1
  [../]
  [./stress_xy]
    type = MaterialTensorAux
    tensor = stress
    variable = stress_xy
    index = 3
  [../]
  [./inc_slip_x]
    type = PenetrationAux
    variable = inc_slip_x
    execute_on = timestep_end
    boundary = 3
    paired_boundary = 2
  [../]
  [./inc_slip_y]
    type = PenetrationAux
    variable = inc_slip_y
    execute_on = timestep_end
    boundary = 3
    paired_boundary = 2
  [../]
  [./accum_slip_x]
    type = PenetrationAux
    variable = accum_slip_x
    execute_on = timestep_end
    boundary = 3
    paired_boundary = 2
  [../]
  [./accum_slip_y]
    type = PenetrationAux
    variable = accum_slip_y
    execute_on = timestep_end
    boundary = 3
    paired_boundary = 2
  [../]
  [./tang_force_x]
    type = PenetrationAux
    variable = tang_force_x
    quantity = tangential_force_x
    boundary = 3
    paired_boundary = 2
  [../]
  [./tang_force_y]
    type = PenetrationAux
    variable = tang_force_y
    quantity = tangential_force_y
    boundary = 3
    paired_boundary = 2
  [../]
  [./penetration]
    type = PenetrationAux
    variable = penetration
    boundary = 3
    paired_boundary = 2
  [../]
[]

[Postprocessors]
  [./bot_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 1
  [../]
  [./bot_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 1
  [../]
  [./top_react_x]
    type = NodalSum
    variable = saved_x
    boundary = 4
  [../]
  [./top_react_y]
    type = NodalSum
    variable = saved_y
    boundary = 4
  [../]
  [./disp_x226]
    type = NodalVariableValue
    nodeid = 225
    variable = disp_x
  [../]
  [./disp_y226]
    type = NodalVariableValue
    nodeid = 225
    variable = disp_y
  [../]
  [./_dt]
    type = TimestepSize
  [../]
  [./num_lin_it]
    type = NumLinearIterations
  [../]
  [./num_nonlin_it]
    type = NumNonlinearIterations
  [../]
[]

[BCs]
  [./side_x]
    type = DirichletBC
    variable = disp_y
    boundary = '1 2'
    value = 0.0
  [../]
  [./bot_y]
    type = DirichletBC
    variable = disp_x
    boundary = '1 2'
    value = 0.0
  [../]
  [./top_y_disp]
    type = FunctionPresetBC
    variable = disp_y
    boundary = 4
    function = disp_ramp_vert
  [../]
  [./top_x_disp]
    type = FunctionPresetBC
    variable = disp_x
    boundary = 4
    function = disp_ramp_horz
  [../]
[]

[Materials]
  [./stiffStuff1]
    type = Elastic
    block = 1
    disp_x = disp_x
    disp_y = disp_y
    youngs_modulus = 1e10
    poissons_ratio = 0.0
    formulation = NonlinearPlaneStrain
  [../]
  [./stiffStuff2]
    type = Elastic
    block = 2
    disp_x = disp_x
    disp_y = disp_y
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    formulation = NonlinearPlaneStrain
  [../]
  [./stiffStuff3]
    type = Elastic
    block = 3
    disp_x = disp_x
    disp_y = disp_y
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    formulation = NonlinearPlaneStrain
  [../]
  [./stiffStuff4]
    type = Elastic
    block = 4
    disp_x = disp_x
    disp_y = disp_y
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    formulation = NonlinearPlaneStrain
  [../]
  [./stiffStuff5]
    type = Elastic
    block = 5
    disp_x = disp_x
    disp_y = disp_y
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    formulation = NonlinearPlaneStrain
  [../]
  [./stiffStuff6]
    type = Elastic
    block = 6
    disp_x = disp_x
    disp_y = disp_y
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    formulation = NonlinearPlaneStrain
  [../]
  [./stiffStuff7]
    type = Elastic
    block = 7
    disp_x = disp_x
    disp_y = disp_y
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    formulation = NonlinearPlaneStrain
  [../]
[]

[Executioner]
  type = Transient

  #Preconditioned JFNK (default)
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  line_search = 'none'

  nl_abs_tol = 1e-5
  nl_rel_tol = 1e-4
  l_max_its = 20
  nl_max_its = 200

  start_time = 0.0
  end_time = 6.0
  l_tol = 1e-6
  dt = 0.2
  dtmin = 0.01
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    petsc_options_iname = 'pc_type'
    petsc_options_value = 'lu'
  [../]
[]

[VectorPostprocessors]
  [./x_disp]
    type = NodalValueSampler
    variable = disp_x
    boundary = '3 4'
    sort_by = id
  [../]
  [./y_disp]
    type = NodalValueSampler
    variable = disp_y
    boundary = '3 4'
    sort_by = id
  [../]
  [./cont_press]
    type = NodalValueSampler
    variable = contact_pressure
    boundary = '3'
    sort_by = id
  [../]
[]

[Outputs]
  print_linear_residuals = true
  print_perf_log = true
  [./exodus]
    type = Exodus
    elemental_as_nodal = true
  [../]
  [./console]
    type = Console
    max_rows = 5
  [../]
  [./chkfile]
    type = CSV
    show = 'x_disp y_disp cont_press'
    start_time = 0.9
    execute_vector_postprocessors_on = timestep_end
  [../]
  [./chkfile2]
    type = CSV
    show = 'bot_react_x bot_react_y disp_x226 disp_y226 top_react_x top_react_y'
    start_time = 0.9
    execute_vector_postprocessors_on = timestep_end
  [../]
  [./outfile]
    type = CSV
    delimiter = ' '
    execute_vector_postprocessors_on = none
  [../]
[]

[Contact]
  [./interface]
    master = 2
    slave = 3
    formulation = augmented_lagrange
    system = constraint
    normalize_penalty = true
    penalty = 1e8
    model = coulomb
    friction_coefficient = 0.0
    tangential_tolerance = 1e-3
  [../]
[]
