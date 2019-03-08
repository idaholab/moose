mu=4e-3
rho=1

[GlobalParams]
  gravity = '0 0 0'
  supg = true
  pspg = true
  convective_term = true
  integrate_p_by_parts = true
  transient_term = true
  laplace = true
  u = vel_x
  v = vel_y
  w = vel_z
  p = p
  alpha = 1e0
  order = FIRST
  family = LAGRANGE
[]

[Mesh]
  # file = first_order_flow_refined.msh
  file = sphere_hybrid.e
[]

[Variables]
  [./vel_x]
  [../]

  [./vel_y]
  [../]

  [./vel_z]
  [../]

  [./p]
  [../]
[]

[Kernels]
  # mass
  [./mass]
    type = INSMass
    variable = p
  [../]

  [./x_time]
    type = INSMomentumTimeDerivative
    variable = vel_x
  [../]
  [./y_time]
    type = INSMomentumTimeDerivative
    variable = vel_y
  [../]
  [./z_time]
    type = INSMomentumTimeDerivative
    variable = vel_z
  [../]

  # x-momentum, space
  [./x_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_x
    component = 0
  [../]

  # y-momentum, space
  [./y_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_y
    component = 1
  [../]

  # z-momentum, space
  [./z_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_z
    component = 2
  [../]
[]

[BCs]
  [./vel_x_no_slip]
    type = DirichletBC
    boundary = 'no_slip inlet'
    variable = vel_x
    value = 0
  [../]
  [./vel_y_no_slip]
    type = DirichletBC
    boundary = 'no_slip inlet'
    variable = vel_y
    value = 0
  [../]
  [./vel_z_no_slip]
    type = DirichletBC
    boundary = 'no_slip'
    variable = vel_z
    value = 0
  [../]
  [./vel_z_inlet]
    type = FunctionDirichletBC
    function = inlet_func
    variable = vel_z
    boundary = inlet
  [../]
[]

[Functions]
  [./inlet_func]
    type = ParsedFunction
    value = 'sqrt((x-2)^2 * (x+2)^2 * (y-2)^2 * (y+2)^2) / 16'
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    prop_names = 'rho mu'
    prop_values = '${rho}  ${mu}'
  [../]
[]

[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    solve_type = 'NEWTON'
  [../]
[]

[Executioner]
  # Note: -snes_ksp_ew seems to lead to more nonlinear iterations, which isn't ideal
  # when compute_jacobian() is so expensive for this problem.
  petsc_options = '-snes_converged_reason -ksp_converged_reason'

  # Direct solver
  # petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_mat_solver_package'
  # petsc_options_value = 'lu NONZERO superlu_dist'

  # ASM(m) + ILU(n)
  # petsc_options_iname = '-pc_type -pc_asm_overlap -sub_pc_type -sub_pc_factor_levels'
  # petsc_options_value = 'asm      1               ilu          4'

  # Block Jacobi + ILU(n)
  petsc_options_iname = '-pc_type -sub_pc_type -sub_pc_factor_levels'
  petsc_options_value = 'bjacobi  ilu          2'

  line_search = 'none'
  nl_rel_tol = 1e-8
  nl_abs_tol = 1e-12
  nl_max_its = 10
  l_tol = 1e-6
  l_max_its = 250
  # To run to steady-state, set num-steps to some large number (1000000 for example)
  type = Transient
  num_steps = 5
  trans_ss_check = true
  ss_check_tol = 1e-10
  dtmin = 5e-4
  dt = .5
  # For timing studies, turn off IterationAdaptiveDT. Otherwise you can get different
  # solve histories with different PETSc options, and apples-to-oranges comparisons.
  # [./TimeStepper]
  #   dt = .5
  #   type = IterationAdaptiveDT
  #   cutback_factor = 0.4
  #   growth_factor = 1.2
  #   optimal_iterations = 5
  # [../]
[]

[Outputs]
  execute_on = 'timestep_end initial'
  print_perf_log = true
  [./exodus]
    type = Exodus
  [../]
  [./csv]
    type = CSV
  [../]
[]

[AuxVariables]
  [./vxx]
    family = MONOMIAL
    order = FIRST
  [../]
[]

[AuxKernels]
  [./vxx]
    type = VariableGradientComponent
    component = x
    variable = vxx
    gradient_variable = vel_x
  [../]
[]
