[GlobalParams]
  displacements = 'disp_x disp_y'
  D_name = 1e6
[]

[Mesh]
  file = two-body-no-sep-4elem-blocks.e
[]

[Problem]
  kernel_coverage_check = false
[]

[Variables]
  [./disp_x]
    block = '1 2'
  [../]
  [./disp_y]
    block = '1 2'
  [../]
  [./lm]
    block = 3
  [../]
  [./vel_x]
    block = 2
  [../]
  [./vel_y]
    block = 2
  [../]
[]

[ICs]
  [./block2y]
    block = 2
    variable = disp_y
    type = ConstantIC
    value = -.1
  [../]
[]

[Kernels]
  [./disp_x]
    type = MatDiffusion
    variable = disp_x
  [../]
  [./disp_y]
    type = MatDiffusion
    variable = disp_y
  [../]
  [./accel_x]
    type = CoupledTimeDerivative
    variable = disp_x
    v = vel_x
    block = 2
  [../]
  [./accel_y]
    type = CoupledTimeDerivative
    variable = disp_y
    v = vel_y
    block = 2
  [../]
  [./coupled_time_velx]
    type = CoupledTimeDerivative
    variable = vel_x
    v = disp_x
    block = 2
  [../]
  [./coupled_time_vely]
    type = CoupledTimeDerivative
    variable = vel_y
    v = disp_y
    block = 2
  [../]
  [./source_velx]
    type = MatReaction
    variable = vel_x
    mob_name = 1
    block = 2
  [../]
  [./source_vely]
    type = MatReaction
    variable = vel_y
    mob_name = 1
    block = 2
  [../]
[]

[Constraints]
  [./lm]
    type = LMConstraint
    slave = 10
    master = 20
    variable = lm
    master_variable = disp_x
    disp_y = disp_y
    use_displaced_mesh = true
  [../]
[]

[BCs]
  [./botx]
    type = DirichletBC
    variable = disp_x
    boundary = 40
    value = 0.0
  [../]
  [./boty]
    type = DirichletBC
    variable = disp_y
    boundary = 40
    value = 0.0
  [../]
  [./topy]
    type = NeumannBC
    variable = disp_y
    boundary = 30
    value = -1
  [../]
[]

[Executioner]
  type = Transient
  num_steps = 2
  dt = 1
  dtmin = 1
  solve_type = 'NEWTON'
  line_search = 'basic'
  petsc_options = '-snes_converged_reason -ksp_converged_reason -pc_svd_monitor -snes_test_jacobian'# -snes_test_jacobian_view'
  petsc_options_iname = '-pc_type'
  petsc_options_value = 'svd'

  l_max_its = 30
  nl_max_its = 20
[]

[Outputs]
  exodus = true
  [./dofmap]
    type = DOFMap
    execute_on = 'initial'
  [../]
[]

[Contact]
  [./leftright]
    master = 20
    slave = 10
    model = frictionless
    formulation = lagrange
    system = constraint
    lm = lm
  [../]
[]

[Debug]
  show_var_residual_norms = true
[]

[Preconditioning]
  [./smp]
    type = SMP
    full = true
  [../]
[]
