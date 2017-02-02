#
# Diffusion with neighbor relation of quasiperiodicity
# using the lagrange multiplier approach
#

[Mesh]
  type = GeneratedMesh
  dim = 2
  nx = 2
  ny = 2
  xmin = -0.05
  xmax = 0.05
  ymin = -0.05
  ymax = 0.05
[]

[GlobalParams]
  use_displaced_mesh = false
  jacobian_fill = 0
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
[]


[InterfaceKernels]
  [./iface_x_l]
    type = InterfaceDiffusionBoundaryTerm
    boundary = 'left'
    variable = u
    neighbor_var = u
  [../]


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

  dt = 0.001
  dtmax = 0.001
  dtmin = 0.001

  num_steps = 1
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
