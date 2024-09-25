#  This is a benchmark test that checks constraint based frictionless
#  contact using the penalty method.  In this test a sinusoidal
#  displacement is applied in the horizontal direction to simulate
#  a small block come in and out of contact as it slides down a larger block.
#
#  The sinusoid is of the form 0.4sin(4t)+0.2. The gold file is run
#  on one processor and the benchmark
#  case is run on a minimum of 4 processors to ensure no parallel variability
#  in the contact pressure and penetration results.  Further documentation can
#  found in moose/modules/contact/doc/sliding_block/
#

[Mesh]
  file = sliding_elastic_blocks_2d.e
  patch_size = 80
[]

[GlobalParams]
  volumetric_locking_correction = false
  displacements = 'disp_x disp_y'
[]

[AuxVariables]
  [./penetration]
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
  [./vertical_movement]
    type = ParsedFunction
    expression = -t
  [../]
  [./horizontal_movement]
    type = ParsedFunction
    expression = -0.04*sin(4*t)+0.02
  [../]
[]

[Physics/SolidMechanics/QuasiStatic]
  [./all]
    add_variables = true
    strain = FINITE
  [../]
[]

[AuxKernels]
  [./zeroslip_x]
    type = ConstantAux
    variable = inc_slip_x
    boundary = 3
    execute_on = timestep_begin
    value = 0.0
  [../]
  [./zeroslip_y]
    type = ConstantAux
    variable = inc_slip_y
    boundary = 3
    execute_on = timestep_begin
    value = 0.0
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
  [./nonlinear_its]
    type = NumNonlinearIterations
    execute_on = timestep_end
  [../]
  [./penetration]
    type = NodalVariableValue
    variable = penetration
    nodeid = 222
  [../]
  [./contact_pressure]
    type = NodalVariableValue
    variable = contact_pressure
    nodeid = 222
  [../]
  [./tot_nonlin_it]
    type = CumulativeValuePostprocessor
    postprocessor = nonlinear_its
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
    type = FunctionDirichletBC
    variable = disp_x
    boundary = 4
    function = horizontal_movement
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
    type = ComputeIsotropicElasticityTensor
    block = '1 2'
    youngs_modulus = 1e6
    poissons_ratio = 0.3
    constant_on = SUBDOMAIN
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
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

  petsc_options = '-ksp_monitor_true_residual'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu     superlu_dist'

  line_search = 'contact'
  contact_line_search_ltol = .5
  contact_line_search_allowed_lambda_cuts = 0

  l_max_its = 100
  nl_max_its = 20
  dt = 0.1
  end_time = 3
  # num_steps = 30
  l_tol = 1e-6
  nl_rel_tol = 1e-10
  nl_abs_tol = 1e-6
  dtmin = 0.01
[]

[Outputs]
  perf_graph = true
  print_linear_residuals = false
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
    primary = 2
    model = frictionless
    penalty = 1e+7
    formulation = penalty
    normal_smoothing_distance = 0.1
  [../]
[]
