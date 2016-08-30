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
  type = Steady
  petsc_options_iname = '-pc_type -pc_factor_shift_type -pc_factor_shift_amount -ksp_type'
  petsc_options_value = 'lu NONZERO 1.e-10 preonly'
  nl_rel_tol = 1e-12
  nl_max_its = 6
[]

[Outputs]
  [./out]
    type = Exodus
  []
[]

[Variables]
  # Velocity in radial direction
  [./u]
    family = LAGRANGE
    order = SECOND
  [../]
  # Velocity in axial direction
  [./v]
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
    variable = u
    value = 0
  [../]
  [./v_in]
    type = FunctionDirichletBC
    boundary = bottom
    variable = v
    function = 'inlet_func'
  [../]
  [./u_out]
    type = INSMomentumNoBCBC
    boundary = top
    variable = u
    u = u
    v = v
    p = p
    component = 0
  [../]
  [./v_out]
    type = INSMomentumNoBCBC
    boundary = top
    variable = v
    u = u
    v = v
    p = p
    component = 1
  [../]
  [./u_axis_and_walls]
    type = DirichletBC
    boundary = 'left right'
    variable = u
    value = 0
  [../]
  [./v_no_slip]
    type = DirichletBC
    boundary = 'right'
    variable = v
    value = 0
  [../]
[]


[Kernels]
  # mass
  [./mass]
    type = INSMass
    variable = p
    u = u
    v = v
    p = p
  [../]
  [./x_momentum_space]
    type = INSMomentum
    variable = u
    u = u
    v = v
    p = p
    component = 0
  [../]
  # z-momentum, space
  [./y_momentum_space]
    type = INSMomentum
    variable = v
    u = u
    v = v
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
