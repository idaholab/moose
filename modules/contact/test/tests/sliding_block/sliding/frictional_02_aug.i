#  This is a benchmark test that checks constraint based frictional
#  contact using the augmented lagrangian method.  In this test a constant
#  displacement is applied in the horizontal direction to simulate
#  a small block come sliding down a larger block.
#
#  A friction coefficient of 0.2 is used.  The gold file is run on one processor
#  and the benchmark case is run on a minimum of 4 processors to ensure no
#  parallel variability in the contact pressure and penetration results.
#

[Mesh]
  file = sliding_elastic_blocks_2d.e
  patch_size = 80
[]

[GlobalParams]
  displacements = 'disp_x disp_y'
  volumetric_locking_correction = false
[]

[AuxVariables]
  [./contact_traction]
  [../]
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
  [./saved_x]
  [../]
  [./saved_y]
  [../]
  [./diag_saved_x]
  [../]
  [./diag_saved_y]
  [../]
  [./tang_force_x]
  [../]
  [./tang_force_y]
  [../]
[]

[Functions]
  [./vertical_movement]
    type = ParsedFunction
    expression = -t
  [../]
[]

[Modules/TensorMechanics/Master]
  [./all]
    add_variables = true
    strain = FINITE
    save_in = 'saved_x saved_y'
    extra_vector_tags = 'ref'
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
    value = -0.02
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
  [../]
  [./stress]
    type = ComputeFiniteStrainElasticStress
    block = '1 2'
  [../]
[]

[Executioner]
  type = Transient
  solve_type = 'PJFNK'

  petsc_options = '-snes_ksp_ew'
  petsc_options_iname = '-pc_type -pc_factor_mat_solver_type'
  petsc_options_value = 'lu     superlu_dist'

  line_search = 'none'

  l_max_its = 20
  nl_max_its = 200
  dt = 0.1
  end_time = 15
  num_steps = 200
  l_tol = 1e-6
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-8
  dtmin = 0.01

  [./Predictor]
    type = SimplePredictor
    scale = 1.0
  [../]
[]

[Outputs]
  interval = 10
  [./out]
    type = Exodus
    elemental_as_nodal = true
  [../]
  [./console]
    type = Console
    max_rows = 5
  [../]
[]

[Problem]
  type = AugmentedLagrangianContactProblem
  solution_variables = 'disp_x disp_y'
  extra_tag_vectors = 'ref'
  reference_vector = 'ref'
  maximum_lagrangian_update_iterations = 100
[]

[Contact]
  [./leftright]
    secondary = 3
    primary = 2
    model = coulomb
    penalty = 1e+7
    friction_coefficient = 0.2
    formulation = augmented_lagrange
    normalize_penalty = true
    al_penetration_tolerance = 1e-6
    al_incremental_slip_tolerance = 1.0e-2
    al_frictional_force_tolerance =  1e-3
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
  [../]
[]
