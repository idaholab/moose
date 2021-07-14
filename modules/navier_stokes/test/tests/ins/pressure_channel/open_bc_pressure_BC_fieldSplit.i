# This input file tests Dirichlet pressure in/outflow boundary conditions for the incompressible NS equations.
[GlobalParams]
  gravity = '0 0 0'
[]

[Mesh]
  type = GeneratedMesh
  dim = 2
  xmin = 0
  xmax = 3.0
  ymin = 0
  ymax = 1.0
  nx = 30
  ny = 10
  elem_type = QUAD9
[]

[Variables]
  [./vel_x]
    order = SECOND
    family = LAGRANGE
  [../]
  [./vel_y]
    order = SECOND
    family = LAGRANGE
  [../]
  [./p]
    order = FIRST
    family = LAGRANGE
  [../]
[]

[Kernels]
  [./mass]
    type = INSMass
    variable = p
    u = vel_x
    v = vel_y
    pressure = p
  [../]
  [./x_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_x
    u = vel_x
    v = vel_y
    pressure = p
    component = 0
    integrate_p_by_parts = false
  [../]
  [./y_momentum_space]
    type = INSMomentumLaplaceForm
    variable = vel_y
    u = vel_x
    v = vel_y
    pressure = p
    component = 1
    integrate_p_by_parts = false
  [../]
[]

[BCs]
  [./x_no_slip]
    type = DirichletBC
    variable = vel_x
    boundary = 'top bottom'
    value = 0.0
  [../]
  [./y_no_slip]
    type = DirichletBC
    variable = vel_y
    boundary = 'left top bottom'
    value = 0.0
  [../]
  [./inlet_p]
    type = DirichletBC
    variable = p
    boundary = left
    value = 1.0
  [../]
  [./outlet_p]
    type = DirichletBC
    variable = p
    boundary = right
    value = 0.0
  [../]
[]

[Materials]
  [./const]
    type = GenericConstantMaterial
    block = 0
    prop_names = 'rho mu'
    prop_values = '1  1'
  [../]
[]

[Preconditioning]
  active = FSP
  [./FSP]
    type = FSP
    # It is the starting point of splitting
    topsplit = 'up' # 'up' should match the following block name
    [./up]
      splitting = 'u p' # 'u' and 'p' are the names of subsolvers
      splitting_type  = schur
      # Splitting type is set as schur, because the pressure part of Stokes-like systems
      # is not diagonally dominant. CAN NOT use additive, multiplicative and etc.
      # Original system:
      # | A B | | u | = | f_u |
      # | C 0 | | p |   | f_v |
      # is factorized into
      # |I        0 | | A    0|  | I  A^{-1}B | | u | = | f_u |
      # |CA^{-1}  I | | 0   -S|  | 0    I     | | p |   | f_v |
      # S = CA^{-1}B
      # The preconditioning is accomplished via the following steps
      # (1) p^{(0)} = f_v - CA^{-1}f_u,
      # (2) pressure = (-S)^{-1} p^{(0)}
      # (3) u = A^{-1}(f_u-Bp)
      petsc_options_iname = '-pc_fieldsplit_schur_fact_type  -pc_fieldsplit_schur_precondition'
      petsc_options_value = 'full selfp'
      # Factorization type here is full, which means we approximate the original system
      # exactly. There are three other options:
      # diag:
      # | A    0|
      # | 0   -S|
      # lower:
      # |I        0  |
      # |CA^{-1}  -S |
      # upper:
      # | I  A^{-1}B |
      # | 0    -S    |
      # The preconditioning matrix is set as selfp, which means we explicitly form a
      # matrix \hat{S} = C(diag(A))^{-1}B. We do not compute the inverse of A, but instead, we compute
      # the inverse of diag(A).
    [../]
    [./u]
      vars = 'vel_x vel_y'
      # PETSc options for this subsolver
      # A prefix will be applied, so just put the options for this subsolver only
      petsc_options_iname = '-pc_type -ksp_type -ksp_rtol'
      petsc_options_value = '     hypre gmres 1e-4'
      # Specify options to solve A^{-1} in the steps (1), (2) and (3).
      # Solvers for A^{-1} could be different in different steps. We could
      # choose in the following pressure block.
    [../]
    [./p]
      vars = 'p'
      # PETSc options for this subsolver in the step (2)
      petsc_options_iname = '-pc_type -ksp_type -ksp_rtol'
      petsc_options_value = '   jacobi    gmres     1e-4'
      # Use -inner_ksp_type and -inner_pc_type to override A^{-1} in the step (2)
      # Use -lower_ksp_type and -lower_pc_type to override A^{-1} in the step (1)
    [../]
  [../]
[]

[Executioner]
  type = Steady
  solve_type = PJFNK
  nl_rel_tol = 1e-12
  nl_max_its = 6
  l_tol = 1e-6
  l_max_its = 300
[]

[Outputs]
  file_base = open_bc_out_pressure_BC_fieldSplit
  exodus = true
[]
