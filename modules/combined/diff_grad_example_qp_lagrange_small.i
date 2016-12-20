#
# Diffusion with neighbor relation of quasiperiodicity
# using the lagrange multiplier approach
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 6
  ny = 6
  xmin = -0.05
  xmax = 0.05
  ymin = -0.05
  ymax = 0.05
[]

[UserObjects]
  [./quasiperiodicneighbors_x]
    type = QuasiPeriodicNeighbors
    execute_on = 'initial linear nonlinear timestep_begin'
    component = 0
  [../]
  [./quasiperiodicneighbors_y]
    type = QuasiPeriodicNeighbors
    execute_on = 'initial linear nonlinear timestep_begin'
    component = 1
  [../]
[]

[GlobalParams]
  #penalty = 1.0
  #D = 1.00
  #D_neighbor = 1.0
  use_displaced_mesh = false
[]

[Variables]
  [./u]
      [./InitialCondition]
        type = FunctionIC
        function = 'r:=(x-0.01)^2+(y-0.01)^2;if(r<0.0005,0.125,0)'
      [../]
    block = 0
  [../]
  [./lambda_lr]
  [../]
  [./lambda_tb]
  [../]
[]

[AuxVariables]
  [./u_grad_x]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./u_grad_y]
    order = CONSTANT
    family = MONOMIAL
  [../]
  [./R_u]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[AuxKernels]
  [./comp_R_u]
    type = DebugResidualAux
    block = 0
    execute_on = 'timestep_end'
    variable = R_u
    debug_variable = u
  [../]
  [./ugrad_xA]
    type = VariableGradientComponent
    variable = u_grad_x
    gradient_variable = u
    component = x
    execute_on = 'initial timestep_end'
  [../]
  [./ugrad_yA]
    type = VariableGradientComponent
    variable = u_grad_y
    gradient_variable = u
    component = y
    execute_on = 'initial timestep_end'
  [../]
[]

[Kernels]
  [./u1_diff]
    type = Diffusion
    variable = u
  [../]
  [./u1_dt]
    type = TimeDerivative
    variable = u
  [../]

  [./null_lambda_lr]
    type = NullKernel
    variable = lambda_lr
  [../]
  [./null_lambda_tb]
    type = NullKernel
    variable = lambda_tb
  [../]
[]


[InterfaceKernels]
  [./iface_x_l]
    type = InterfaceDiffusionBoundaryTerm
    boundary = 'left'
    variable = u
    neighbor_var = u
  [../]
  #apparently only need one of these per pair of boundaries..
  #[./iface_x_r]
  #  type = InterfaceDiffusionBoundaryTerm
  #  boundary = 'right'
  #  variable = u
  #  neighbor_var = u
  #[../]

  [./lambda_x]
    type = EqualGradientLagrangeMultiplier
    variable = lambda_lr
    boundary = 'left'
    element_var = u
    neighbor_var = u
    component = 0
  [../]
  [./constraint_x]
    type = EqualGradientLagrangeInterface
    boundary = 'left'
    lambda = lambda_lr
    variable = u
    neighbor_var = u
    component = 0
  [../]

  [./iface_y_t]
    type = InterfaceDiffusionBoundaryTerm
    boundary = 'top'
    variable = u
    neighbor_var = u
  [../]
  #[./iface_y_b]
  #  type = InterfaceDiffusionBoundaryTerm
  #  boundary = 'bottom'
  #  variable = u
  #  neighbor_var = u
  #[../]
  [./lambda_y]
    type = EqualGradientLagrangeMultiplier
    variable = lambda_tb
    boundary = 'top'
    element_var = u
    neighbor_var = u
    component = 1
  [../]
  [./constraint_y]
    type = EqualGradientLagrangeInterface
    boundary = 'top'
    lambda = lambda_tb
    variable = u
    neighbor_var = u
    component = 1
  [../]
[]


[Preconditioning]
  [./SMP]
    type = SMP
    full = true
    petsc_options = '-snes_converged_reason'
    petsc_options_iname = '-ksp_gmres_restart -pc_type -pc_factor_shift_type'
    petsc_options_value = '150           lu   nonzero'
  [../]
[]

[Executioner]
  type = Transient
  scheme = implicit-euler #rk4 is no less stable
  solve_type = 'PJFNK'

  nl_rel_tol = 1.0e-8
  nl_abs_tol = 1.0e-10

  start_time = 0.0

  dt = 0.0001
  dtmax = 0.0001

  num_steps = 200
[]

[Outputs]
  execute_on = 'timestep_end'
  print_linear_residuals = true
  exodus = true
  file_base = out_test_diff_u_lagrange_int_small
  [./table]
    type = CSV
    delimiter = ' '
  [../]
[]
