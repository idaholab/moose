[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = -1.0
    xmax = 3.0
    ymin = 0.0
    ymax = 1.0
    nx = 1000
    ny = 250
    elem_type = QUAD4
  []
[]
[Problem]
kernel_coverage_check = false
material_coverage_check = false
skip_nl_system_check = true
solve = false
[]


[AuxVariables]
  [indicator]
    order = FIRST
    family = LAGRANGE
    initial_condition = 0.0
  []
[]

[Materials]
[]

[Executioner]
  type = Transient
  solve_type = 'NEWTON'
  # Run for 100+ timesteps to reach stey state.

  num_steps = 1
  dt = 1e-2
  end_time = 1
  dtmin = 1.0e-7
  # Note: -snes_ksp_ew seems to le to more nonlinear iterations, which isn't ideal
  # when compute_jacobian() is so expensive for this problem.
  petsc_options = '-snes_converged_reason -ksp_converged_reason'

  # # Direct solver
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_package'
  petsc_options_value = 'lu NONZERO superlu_dist'

  # petsc_options_iname = '-pc_type -pc_hypre_type'
  # petsc_options_value = 'hypre boomeramg'
  # residual_and_jacobian_together = true
  line_search = 'bt'
  nl_rel_tol = 1e-6
  nl_abs_tol = 2e-7
  nl_max_its = 5
  l_max_its = 25

  automatic_scaling = true
  off_diagonals_in_auto_scaling = true
  compute_scaling_once = false
  # scaling_group_variables = 'velocity p'
  [TimeIntegrator]
    type = BDF2
  []

[]

[Outputs]
  exodus = true
  print_linear_residuals = false
[]

[MultiApps]
  [solid_domain]
    type = TransientMultiApp
    execute_on = "INITIAL TIMESTEP_END"
    # positions = '0 0 0'
    input_files = child.i
    use_displaced_mesh = true
    # sub_cycling = true
    catch_up = true
    max_catch_up_steps = 5
    keep_solution_during_restore = true
  []
[]

[Transfers]
  [push_indicator]
    # type = MultiAppNearestNodeTransfer
    type = MultiAppShapeEvaluationTransfer
    # Transfer from the sub-app from this app
    from_multi_app =  solid_domain
    # The name of the variable in this app
    source_variable = solid_indicator
    # The name of the auxiliary variable in the sub-app
    variable = indicator
    displaced_source_mesh = true
    use_displaced_mesh = true
  []
[]

[Postprocessors]
  [89360]
    type = NodalVariableValue
    nodeid = 89360
    variable = indicator
    execute_on = 'timestep_end final'
  []
  [89361]
    type = NodalVariableValue
    nodeid = 89361
    variable = indicator
    execute_on = 'timestep_end final'
  []
  [89362]
    type = NodalVariableValue
    nodeid = 89362
    variable = indicator
    execute_on = 'timestep_end final'
  []
  [89363]
    type = NodalVariableValue
    nodeid = 89363
    variable = indicator
    execute_on = 'timestep_end final'
  []
  [89364]
    type = NodalVariableValue
    nodeid = 89364
    variable = indicator
    execute_on = 'timestep_end final'
  []
  [89365]
    type = NodalVariableValue
    nodeid = 89365
    variable = indicator
    execute_on = 'timestep_end final'
  []
  [89366]
    type = NodalVariableValue
    nodeid = 89366
    variable = indicator
    execute_on = 'timestep_end final'
  []
  [89367]
    type = NodalVariableValue
    nodeid = 89367
    variable = indicator
    execute_on = 'timestep_end final'
  []
  [89368]
    type = NodalVariableValue
    nodeid = 89368
    variable = indicator
    execute_on = 'timestep_end final'
  []
  [89369]
    type = NodalVariableValue
    nodeid = 89369
    variable = indicator
    execute_on = 'timestep_end final'
  []
[]
