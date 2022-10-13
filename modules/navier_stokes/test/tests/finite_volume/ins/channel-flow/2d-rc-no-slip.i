mu = 1.1
rho = 1.1
advected_interp_method = 'average'
velocity_interp_method = 'rc'

[GlobalParams]
  rhie_chow_user_object = 'rc'
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 10
    ymin = -1
    ymax = 1
    nx = 100
    ny = 20
  []
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 1
  []
  [pressure]
    type = INSFVPressureVariable
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'x'
  []
  [u_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = ${mu}
    momentum_component = 'x'
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []

  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = ${rho}
    momentum_component = 'y'
  []
  [v_viscosity]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = ${mu}
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []
[]

[FVBCs]
  [inlet-u]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_x
    function = '1'
  []
  [inlet-v]
    type = INSFVInletVelocityBC
    boundary = 'left'
    variable = vel_y
    function = '0'
  []
  [walls-u]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = vel_x
    function = 0
  []
  [walls-v]
    type = INSFVNoSlipWallBC
    boundary = 'top bottom'
    variable = vel_y
    function = 0
  []
  [outlet_p]
    type = INSFVOutletPressureBC
    boundary = 'right'
    variable = pressure
    function = '0'
  []
[]

[Executioner]
  type = Steady
  solve_type = 'NEWTON'
  nl_rel_tol = 1e-12
[]

[Preconditioning]
  active = SMP
  [FSP]
    type = FSP
    # It is the starting point of splitting
    topsplit = 'up' # 'up' should match the following block name
    [up]
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
      petsc_options_value = 'full                            selfp'
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
    []
    [u]
      vars = 'vel_x vel_y'
      # PETSc options for this subsolver
      # A prefix will be applied, so just put the options for this subsolver only
      petsc_options_iname = '-pc_type -pc_hypre_type -ksp_type -ksp_rtol'
      petsc_options_value = 'hypre    boomeramg      gmres     1e-4'
      # Specify options to solve A^{-1} in the steps (1), (2) and (3).
      # Solvers for A^{-1} could be different in different steps. We could
      # choose in the following pressure block.
    []
    [p]
      vars = 'pressure'
      # PETSc options for this subsolver in the step (2)
      petsc_options_iname = '-pc_type -ksp_type -ksp_rtol'
      petsc_options_value = 'jacobi   gmres     1e-4'
      # Use -inner_ksp_type and -inner_pc_type to override A^{-1} in the step (2)
      # Use -lower_ksp_type and -lower_pc_type to override A^{-1} in the step (1)
    []
  []
  [SMP]
    type = SMP
    full = true
    petsc_options_iname = '-pc_type -pc_factor_shift_type'
    petsc_options_value = 'lu       NONZERO'
  []
[]

[Outputs]
  exodus = true
  csv = true
[]
