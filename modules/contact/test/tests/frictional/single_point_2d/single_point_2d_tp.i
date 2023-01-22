[Mesh]
  file = single_point_2d.e
[]

[GlobalParams]
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y'
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
[]

[Functions]
  [./horizontal_movement]
    type = ParsedFunction
    expression = t
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    strain = FINITE
    save_in = 'saved_x saved_y'
    diag_save_in = 'diag_saved_x diag_saved_y'
  [../]
[]

[AuxKernels]
  [./incslip_x]
    type = PenetrationAux
    variable = inc_slip_x
    quantity = incremental_slip_x
    boundary = 3
    paired_boundary = 2
  [../]
  [./incslip_y]
    type = PenetrationAux
    variable = inc_slip_y
    quantity = incremental_slip_y
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

[BCs]
  [./botx]
    type = DirichletBC
    variable = disp_x
    boundary = 1
    value = 0.0
  [../]
  [./boty]
    type = DirichletBC
    variable = disp_y
    boundary = 1
    value = 0.0
  [../]
  [./botx2]
    type = DirichletBC
    variable = disp_x
    boundary = 2
    value = 0.0
  [../]
  [./boty2]
    type = DirichletBC
    variable = disp_y
    boundary = 2
    value = 0.0
  [../]
  [./topx]
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 4
    function = horizontal_movement
  [../]
  [./topy]
    type = DirichletBC
    variable = disp_y
    boundary = 4
    value = -0.005
  [../]
[]

[Materials]
  [./bottom]
    type = ComputeIsotropicElasticityTensor
    block = '1'
    youngs_modulus = 1.0e9
    poissons_ratio = 0.3
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  [../]
  [./top]
    type = ComputeIsotropicElasticityTensor
    block = '2'
    youngs_modulus = 1.0e6
    poissons_ratio = 0.3
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

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu    superlu_dist'

  line_search = 'none'

  l_max_its = 100
  nl_max_its = 200
  dt = 0.001
  end_time = 0.01
  num_steps = 1000
  nl_rel_tol = 1e-08
  nl_abs_tol = 1e-08
  dtmin = 0.001
  l_tol = 1e-3
[]

[Outputs]
  exodus = true
  print_linear_residuals = true
  perf_graph = true
  [./console]
    type = Console
    max_rows = 5
  [../]
[]

[Contact]
  [./leftright]
    primary = 2
    secondary = 3
    model = coulomb
    friction_coefficient = '0.25'
    formulation = tangential_penalty
    penalty = 1e10
  [../]
[]

[Dampers]
  [./contact_slip]
    type = ContactSlipDamper
    primary = '2'
    secondary = '3'
  [../]
[]
