[GlobalParams]
  rho = 1
  mu = 1
  integrate_p_by_parts = false
  gravity = '0 0 0'
  coord_type = RZ
[]

[Mesh]
  file = '2d_cone.msh'
[]

[Problem]
[]

[Preconditioning]
  [./SMP_PJFNK]
    type = SMP
    full = true
    solve_type = Newton
  [../]
[]

[Executioner]
  type = Transient
  dt = 0.005
  num_steps = 5
  l_max_its = 299

  # Note: The Steady executioner can be used for this problem, if you
  # drop the INSMomentumTimeDerivative kernels and use the following
  # direct solver options.
  # petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -ksp_type'
  # petsc_options_value = 'lu NONZERO 1.e-10 preonly'

  # "Standard" ILU options
  petsc_options_iname = '-ksp_gmres_restart -pc_type -sub_pc_type -sub_pc_factor_levels'
  petsc_options_value = '300                bjacobi  ilu          4'

  nl_rel_tol = 1e-12
  nl_max_its = 6
[]

[Outputs]
  [./out]
    type = Exodus
  []
[]

[Variables]
  [./vel_x]
    # Velocity in radial (r) direction
    family = LAGRANGE
    order = SECOND
  [../]
  [./vel_y]
    # Velocity in axial (z) direction
    family = LAGRANGE
    order = SECOND
  [../]
  [./p]
    family = LAGRANGE
    order = FIRST
  [../]
[]

[BCs]
  [./p_corner]
    type = DirichletBC
    boundary = top_right
    value = 0
    variable = p
  [../]
  [./u_in]
    type = DirichletBC
    boundary = bottom
    variable = vel_x
    value = 0
  [../]
  [./v_in]
    type = FunctionDirichletBC
    boundary = bottom
    variable = vel_y
    function = 'inlet_func'
  [../]
  [./u_out]
    type = INSMomentumNoBCBC
    boundary = top
    variable = vel_x
    u = vel_x
    v = vel_y
    p = p
    component = 0
  [../]
  [./v_out]
    type = INSMomentumNoBCBC
    boundary = top
    variable = vel_y
    u = vel_x
    v = vel_y
    p = p
    component = 1
  [../]
  [./u_axis_and_walls]
    type = DirichletBC
    boundary = 'left right'
    variable = vel_x
    value = 0
  [../]
  [./v_no_slip]
    type = DirichletBC
    boundary = 'right'
    variable = vel_y
    value = 0
  [../]
[]


[Kernels]
  [./x_momentum_time]
    type = INSMomentumTimeDerivative
    variable = vel_x
  [../]
  [./y_momentum_time]
    type = INSMomentumTimeDerivative
    variable = vel_y
  [../]
  [./mass]
    type = INSMass
    variable = p
    u = vel_x
    v = vel_y
    p = p
  [../]
  [./x_momentum_space]
    type = INSMomentum
    variable = vel_x
    u = vel_x
    v = vel_y
    p = p
    component = 0
  [../]
  [./y_momentum_space]
    type = INSMomentum
    variable = vel_y
    u = vel_x
    v = vel_y
    p = p
    component = 1
  [../]
[]

[Functions]
  [./inlet_func]
    type = ParsedFunction
    value = '-4 * x^2 + 1'
  [../]
[]
