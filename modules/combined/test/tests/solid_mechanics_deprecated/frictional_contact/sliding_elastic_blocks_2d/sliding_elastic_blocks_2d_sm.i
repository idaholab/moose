[Mesh]
  file = sliding_elastic_blocks_2d.e
[]

[GlobalParams]
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y'
[]

[Variables]
  [./disp_x]
  [../]
  [./disp_y]
  [../]
[]

[AuxVariables]
  [./penetration]
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
  [./accum_slip]
  [../]
  [./tang_force_x]
  [../]
  [./tang_force_y]
  [../]
[]

[Functions]
  [./vertical_movement]
    type = ParsedFunction
    value = -t
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
    use_displaced_mesh = true
  [../]
[]

[AuxKernels]
  [./inc_slip_x]
    type = PenetrationAux
    variable = inc_slip_x
    quantity = incremental_slip_x
    boundary = 3
    paired_boundary = 2
  [../]
  [./inc_slip_y]
    type = PenetrationAux
    variable = inc_slip_y
    quantity = incremental_slip_y
    boundary = 3
    paired_boundary = 2
  [../]
  [./accum_slip]
    type = PenetrationAux
    variable = accum_slip
    execute_on = timestep_end
    quantity = accumulated_slip
    boundary = 3
    paired_boundary = 2
  [../]
  [./tangential_force_x]
    type = PenetrationAux
    variable = tang_force_x
    execute_on = timestep_end
    quantity = tangential_force_x
    boundary = 3
    paired_boundary = 2
  [../]
  [./tangential_force_y]
    type = PenetrationAux
    variable = tang_force_y
    execute_on = timestep_end
    quantity = tangential_force_y
    boundary = 3
    paired_boundary = 2
  [../]
  [./accum_slip_x]
    type = AccumulateAux
    variable = accum_slip_x
    accumulate_from_variable = inc_slip_x
    execute_on = timestep_end
  [../]
  [./accum_slip_y]
    type = AccumulateAux
    variable = accum_slip_y
    accumulate_from_variable = inc_slip_y
    execute_on = timestep_end
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
  [./ref_resid_x]
    type = NodalL2Norm
    execute_on = timestep_end
    variable = saved_x
  [../]
  [./ref_resid_y]
    type = NodalL2Norm
    execute_on = timestep_end
    variable = saved_y
  [../]
[]

[BCs]
  [./left_x]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./left_y]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./right_x]
    type = DirichletBC
    variable = disp_x
    boundary = 4
    value = -0.005
  [../]
  [./right_y]
    type = FunctionDirichletBC
    variable = disp_y
    boundary = 4
    function = vertical_movement
  [../]
[]

[Materials]
  [./left]
    type = Elastic
    formulation = NonlinearPlaneStrain
    block = 1
    disp_y = disp_y
    disp_x = disp_x
    poissons_ratio = 0.3
    youngs_modulus = 1e7
  [../]
  [./right]
    type = Elastic
    formulation = NonlinearPlaneStrain
    block = 2
    disp_y = disp_y
    disp_x = disp_x
    poissons_ratio = 0.3
    youngs_modulus = 1e6
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu     superlu_dist'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 1000
  dt = 0.01
  end_time = 0.05
  num_steps = 1000
  nl_rel_tol = 1e-16
  nl_abs_tol = 1e-09
  dtmin = 0.01
  l_tol = 1e-3

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]
[]

[Outputs]
  print_linear_residuals = true
  perf_graph = true
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
  [./console]
    type = Console
    max_rows = 5
  [../]
[]

[Contact]
  [./leftright]
    secondary = 3
    master = 2
    model = coulomb
    friction_coefficient = '0.25'
    penalty = 1e6
  [../]
[]

[Dampers]
  [./contact_slip]
    type = ContactSlipDamper
    secondary = 3
    master = 2
  [../]
[]
