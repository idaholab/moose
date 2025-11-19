mu_ref = 1.2
mu_r = 0.1
mu_x = 0.2
rho_ref = 1.4
rho_r = 0.15
rho_x = 0.25
advected_interp_method = 'average'
velocity_interp_method = 'rc'

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  coord_type = 'RZ'
  rz_coord_axis = x
[]

[GlobalParams]
  rhie_chow_user_object = rc
[]

[Variables]
  [vel_x]
    type = INSFVVelocityVariable
    initial_condition = 0.0
  []
  [vel_y]
    type = INSFVVelocityVariable
    initial_condition = 0.0
  []
  [pressure]
    type = INSFVPressureVariable
    initial_condition = 0.0
  []
  [lambda]
    family = SCALAR
    order = FIRST
  []
[]

[UserObjects]
  [rc]
    type = INSFVRhieChowInterpolator
    u = vel_x
    v = vel_y
    pressure = pressure
  []
[]

[FVKernels]
  [mass]
    type = INSFVMassAdvection
    variable = pressure
    rho = rho_fun
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
  []
  [pressure_pin]
    type = FVPointValueConstraint
    lambda = lambda
    phi0 = 0.125
    point = '0.5 0.5 0'
    variable = pressure
  []

  [u_advection]
    type = INSFVMomentumAdvection
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = rho_fun
    momentum_component = 'x'
  []
  [u_diffusion]
    type = INSFVMomentumDiffusion
    variable = vel_x
    mu = mu_fun
    momentum_component = 'x'
    complete_expansion = true
    u = vel_x
    v = vel_y
  []
  [u_pressure]
    type = INSFVMomentumPressure
    variable = vel_x
    momentum_component = 'x'
    pressure = pressure
  []
  [u_body]
    type = INSFVBodyForce
    variable = vel_x
    functor = forcing_u
    momentum_component = 'x'
  []

  [v_diffusion]
    type = INSFVMomentumDiffusion
    variable = vel_y
    mu = mu_fun
    momentum_component = 'y'
    complete_expansion = true
    u = vel_x
    v = vel_y
  []
  [v_viscous_source]
    type = INSFVMomentumViscousSourceRZ
    variable = vel_y
    mu = mu_fun
    momentum_component = 'y'
    complete_expansion = true
  []
  [v_advection]
    type = INSFVMomentumAdvection
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    velocity_interp_method = ${velocity_interp_method}
    rho = rho_fun
    momentum_component = 'y'
  []
  [v_pressure]
    type = INSFVMomentumPressure
    variable = vel_y
    momentum_component = 'y'
    pressure = pressure
  []
  [v_body]
    type = INSFVBodyForce
    variable = vel_y
    functor = forcing_v
    momentum_component = 'y'
  []
[]

[FVBCs]
  [axis_u]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_x
    u = vel_x
    v = vel_y
    mu = mu_fun
    momentum_component = x
  []
  [axis_v]
    type = INSFVSymmetryVelocityBC
    boundary = 'bottom'
    variable = vel_y
    u = vel_x
    v = vel_y
    mu = mu_fun
    momentum_component = y
  []
  [axis_p]
    type = INSFVSymmetryPressureBC
    boundary = 'bottom'
    variable = pressure
  []
  [dirichlet_u_wall]
    type = INSFVNoSlipWallBC
    boundary = 'left right top'
    variable = vel_x
    function = exact_u
  []
  [dirichlet_v_wall]
    type = INSFVNoSlipWallBC
    boundary = 'left right top'
    variable = vel_y
    function = exact_v
  []
[]

[Functions]
  # --------------------------------------------------
  # Material property functions
  # --------------------------------------------------
  [rho_fun]
    type = ParsedFunction
    symbol_names = 'rho_ref rho_r rho_x'
    symbol_values = '${rho_ref} ${rho_r} ${rho_x}'
    expression = 'rho_ref + rho_x*x + rho_r*y'
  []
  [mu_fun]
    type = ParsedFunction
    symbol_names = 'mu_ref mu_r mu_x'
    symbol_values = '${mu_ref} ${mu_r} ${mu_x}'
    expression = 'mu_ref + mu_x*x + mu_r*y'
  []

  # --------------------------------------------------
  # Convenience reciprocals
  # --------------------------------------------------
  [S]
    type=ParsedFunction
    symbol_names = 'rho_fun'
    symbol_values = 'rho_fun'
    expression='1.0/rho_fun'
  []
  [S2]
    type=ParsedFunction
    symbol_names = 'rho_fun'
    symbol_values = 'rho_fun'
    expression='(1.0/rho_fun)^2'
  []
  [S3]
    type=ParsedFunction
    symbol_names = 'rho_fun'
    symbol_values = 'rho_fun'
    expression='(1.0/rho_fun)^3'
  []

  # --------------------------------------------------
  # Building blocks
  # --------------------------------------------------
  [A]
    type=ParsedFunction
    expression='x^2*(1-x)^2'
  []
  [Ap]
    type=ParsedFunction
    expression='2*x - 6*x^2 + 4*x^3'
  []
  [App]
    type=ParsedFunction
    expression='2 - 12*x + 12*x^2'
  []
  [Appp]
    type=ParsedFunction
    expression='-12 + 24*x'
  []
  [R]
    type=ParsedFunction
    expression='y^3 - 2*y^4 + y^5'
  []
  [Rp]
    type=ParsedFunction
    expression='3*y^2 - 8*y^3 + 5*y^4'
  []
  [Rpp]
    type=ParsedFunction
    expression='6*y - 24*y^2 + 20*y^3'
  []
  [Rppp]
    type=ParsedFunction
    expression='6 - 48*y + 60*y^2'
  []

  # --------------------------------------------------
  # Exact solutions (from mass streamfunction)
  # --------------------------------------------------
  [exact_u]
    type=ParsedFunction
    symbol_names = 'A Rp S'
    symbol_values = 'A Rp S'
    expression='A*Rp*S / y'
  []
  [exact_v]
    type=ParsedFunction
    symbol_names = 'Ap R S'
    symbol_values = 'Ap R S'
    expression='-Ap*R*S / y'
  []
  [exact_p]
    type=ParsedFunction
    expression='x*y^2'
  []

  # --------------------------------------------------
  # First derivatives
  # --------------------------------------------------
  [ux]
    type=ParsedFunction
    symbol_names = 'Ap Rp A S rho_x S2'
    symbol_values = 'Ap Rp A S ${rho_x} S2'
    expression='(Ap*Rp*S - A*Rp*rho_x*S2)/y'
  []
  [ur]
    type=ParsedFunction
    symbol_names = 'A Rpp Rp S rho_r S2'
    symbol_values = 'A Rpp Rp S ${rho_r} S2'
    expression='(A*Rpp*S - A*Rp*rho_r*S2)/y - (A*Rp*S)/y^2'
  []
  [vx]
    type=ParsedFunction
    symbol_names = 'App R Ap S rho_x S2'
    symbol_values = 'App R Ap S ${rho_x} S2'
    expression='-(App*R*S - Ap*R*rho_x*S2)/y'
  []
  [vr]
    type=ParsedFunction
    symbol_names = 'Ap Rp R S rho_r S2'
    symbol_values = 'Ap Rp R S ${rho_r} S2'
    expression='(-Ap*Rp*S + Ap*R*rho_r*S2)/y + (Ap*R*S)/y^2'
  []

  # --------------------------------------------------
  # Divergence (for stress)
  # --------------------------------------------------
  [theta]
    type=ParsedFunction
    symbol_names = 'A Rp Ap R rho_x rho_r S2'
    symbol_values = 'A Rp Ap R ${rho_x} ${rho_r} S2'
    expression='( -A*Rp*rho_x + Ap*R*rho_r )*S2 / y'
  []

  [T]
    type=ParsedFunction
    symbol_names = 'A Rp Ap R rho_x rho_r'
    symbol_values = 'A Rp Ap R ${rho_x} ${rho_r}'
    expression='-A*Rp*rho_x + Ap*R*rho_r'
  []
  [thetax]
    type=ParsedFunction
    symbol_names = 'Ap Rp App R rho_x rho_r S2 S3 T'
    symbol_values = 'Ap Rp App R ${rho_x} ${rho_r} S2 S3 T'
    expression='( (-Ap*Rp*rho_x + App*R*rho_r)*S2 - 2*rho_x*T*S3 )/y'
  []
  [thetar]
    type=ParsedFunction
    symbol_names = 'A Rpp Ap Rp rho_x rho_r S2 S3 T'
    symbol_values = 'A Rpp Ap Rp ${rho_x} ${rho_r} S2 S3 T'
    expression='( (-A*Rpp*rho_x + Ap*Rp*rho_r)*S2 - 2*rho_r*T*S3 )/y - (T*S2)/y^2'
  []

  # --------------------------------------------------
  # Second derivatives
  # --------------------------------------------------
  [uxx]
    type=ParsedFunction
    symbol_names = 'App Rp Ap rho_x A S S2 S3'
    symbol_values = 'App Rp Ap ${rho_x} A S S2 S3'
    expression='( App*Rp*S - 2*Ap*Rp*rho_x*S2 + 2*A*Rp*(rho_x^2)*S3 )/y'
  []
  [uxr]
    type=ParsedFunction
    symbol_names = 'Ap Rpp Rp rho_r rho_x A S S2 S3'
    symbol_values = 'Ap Rpp Rp ${rho_r} ${rho_x} A S S2 S3'
    expression='( Ap*Rpp*S - Ap*Rp*rho_r*S2 - A*Rpp*rho_x*S2 + 2*A*Rp*(rho_x*rho_r)*S3 )/y - (Ap*Rp*S - A*Rp*rho_x*S2)/y^2'
  []
  [urr]
    type=ParsedFunction
    symbol_names = 'A Rppp Rpp Rp rho_r S S2 S3'
    symbol_values = 'A Rppp Rpp Rp ${rho_r} S S2 S3'
    expression='( A*Rppp*S - 2*A*Rpp*rho_r*S2 + 2*A*Rp*(rho_r^2)*S3 )/y - 2*(A*Rpp*S - A*Rp*rho_r*S2)/y^2 + 2*(A*Rp*S)/y^3'
  []
  [vxx]
    type=ParsedFunction
    symbol_names = 'Appp R App rho_x Ap S S2 S3'
    symbol_values = 'Appp R App ${rho_x} Ap S S2 S3'
    expression='( -Appp*R*S + 2*App*R*rho_x*S2 - 2*Ap*R*(rho_x^2)*S3 )/y'
  []
  [vxr]
    type=ParsedFunction
    symbol_names = 'App Rp R rho_r rho_x Ap S S2 S3'
    symbol_values = 'App Rp R ${rho_r} ${rho_x} Ap S S2 S3'
    expression='( -App*Rp*S + App*R*rho_r*S2 + Ap*Rp*rho_x*S2 - 2*Ap*R*(rho_x*rho_r)*S3 )/y + (App*R*S - Ap*R*rho_x*S2)/y^2'
  []
  [vrr]
    type=ParsedFunction
    symbol_names = 'Ap Rpp R Rp rho_r S S2 S3'
    symbol_values = 'Ap Rpp R Rp ${rho_r} S S2 S3'
    expression='( -Ap*Rpp*S + 2*Ap*Rp*rho_r*S2 - 2*Ap*R*(rho_r^2)*S3 )/y + 2*(Ap*Rp*S - Ap*R*rho_r*S2)/y^2 - 2*(Ap*R*S)/y^3'
  []

  # --------------------------------------------------
  # Stresses
  # --------------------------------------------------
  [tau_xx]
    type=ParsedFunction
    symbol_names = 'mu_fun ux'
    symbol_values = 'mu_fun ux'
    expression='2*mu_fun*ux'
  []
  [tau_rr]
    type=ParsedFunction
    symbol_names = 'mu_fun vr'
    symbol_values = 'mu_fun vr'
    expression='2*mu_fun*vr'
  []
  [tau_xr]
    type=ParsedFunction
    symbol_names = 'mu_fun ur vx'
    symbol_values = 'mu_fun ur vx'
    expression='mu_fun*(ur + vx)'
  []
  [tau_tt]
    type=ParsedFunction
    symbol_names = 'mu_fun exact_v'
    symbol_values = 'mu_fun exact_v'
    expression='2*mu_fun*(exact_v/y)'
  []

  # --------------------------------------------------
  # Stress derivatives (for symmetric-only stress)
  # --------------------------------------------------
  [tau_xx_x]
    type = ParsedFunction
    symbol_names  = 'mu_fun ux uxx mu_x'
    symbol_values = 'mu_fun ux uxx ${mu_x}'
    expression = '2*(mu_x*ux + mu_fun*uxx)'
  []

  [tau_xr_x]
    type = ParsedFunction
    symbol_names  = 'mu_fun ur vx uxr vxx mu_x'
    symbol_values = 'mu_fun ur vx uxr vxx ${mu_x}'
    expression = 'mu_x*(ur + vx) + mu_fun*(uxr + vxx)'
  []

  [tau_xr_r]
    type = ParsedFunction
    symbol_names  = 'mu_fun ur vx urr vxr mu_r'
    symbol_values = 'mu_fun ur vx urr vxr ${mu_r}'
    expression = 'mu_r*(ur + vx) + mu_fun*(urr + vxr)'
  []

  [tau_rr_r]
    type = ParsedFunction
    symbol_names  = 'mu_fun vr vrr mu_r'
    symbol_values = 'mu_fun vr vrr ${mu_r}'
    expression = '2*(mu_r*vr + mu_fun*vrr)'
  []

  # --------------------------------------------------
  # Convective fluxes
  # --------------------------------------------------
  [Fxx]
    type=ParsedFunction
    symbol_names = 'rho_fun exact_u'
    symbol_values = 'rho_fun exact_u'
    expression='rho_fun*exact_u*exact_u'
  []
  [Fxr]
    type=ParsedFunction
    symbol_names = 'rho_fun exact_u exact_v'
    symbol_values = 'rho_fun exact_u exact_v'
    expression='rho_fun*exact_u*exact_v'
  []
  [Frx]
    type=ParsedFunction
    symbol_names = 'rho_fun exact_u exact_v'
    symbol_values = 'rho_fun exact_u exact_v'
    expression='rho_fun*exact_v*exact_u'
  []
  [Frr]
    type=ParsedFunction
    symbol_names = 'rho_fun exact_v'
    symbol_values = 'rho_fun exact_v'
    expression='rho_fun*exact_v*exact_v'
  []

  # --------------------------------------------------
  # Conservative flux derivatives (product rule)
  # --------------------------------------------------
  [Fxx_x]
    type = ParsedFunction
    symbol_names  = 'rho_fun exact_u ux rho_x'
    symbol_values = 'rho_fun exact_u ux ${rho_x}'
    expression = 'rho_x*exact_u*exact_u + rho_fun*(2*exact_u*ux)'
  []

  [Frx_r]
    type = ParsedFunction
    symbol_names  = 'rho_fun exact_u exact_v ur vr rho_r'
    symbol_values = 'rho_fun exact_u exact_v ur vr ${rho_r}'
    expression = 'rho_r*exact_v*exact_u + rho_fun*(vr*exact_u + exact_v*ur)'
  []

  [Fxr_x]
    type = ParsedFunction
    symbol_names  = 'rho_fun exact_u exact_v ux vx rho_x'
    symbol_values = 'rho_fun exact_u exact_v ux vx ${rho_x}'
    expression = 'rho_x*exact_u*exact_v + rho_fun*(ux*exact_v + exact_u*vx)'
  []

  [Frr_r]
    type = ParsedFunction
    symbol_names  = 'rho_fun exact_v vr rho_r'
    symbol_values = 'rho_fun exact_v vr ${rho_r}'
    expression = 'rho_r*exact_v*exact_v + rho_fun*(2*exact_v*vr)'
  []

  # --------------------------------------------------
  # Pressure gradients
  # --------------------------------------------------
  [px]
    type=ParsedFunction
    expression='y^2'
  []
  [pr]
    type=ParsedFunction
    expression='2*x*y'
  []

  # --------------------------------------------------
  # Forcing terms (momentum residuals)
  # --------------------------------------------------
  [forcing_u]
    type=ParsedFunction
    symbol_names = 'Fxx_x Frx Frx_r px tau_xx_x tau_xr tau_xr_r'
    symbol_values = 'Fxx_x Frx Frx_r px tau_xx_x tau_xr tau_xr_r'
    expression='Fxx_x + (Frx/y) + Frx_r + px - ( tau_xx_x + (tau_xr/y) + tau_xr_r )'
  []
  [forcing_v]
    type=ParsedFunction
    symbol_names = 'Fxr_x Frr Frr_r pr tau_xr_x tau_rr tau_rr_r tau_tt'
    symbol_values = 'Fxr_x Frr Frr_r pr tau_xr_x tau_rr tau_rr_r tau_tt'
    expression='Fxr_x + (Frr/y) + Frr_r + pr - ( tau_xr_x + (tau_rr/y) + tau_rr_r - (tau_tt)/y )'
  []
[]

[AuxVariables]
  [vel_x_aux]
    type = MooseLinearVariableFVReal
    initial_condition = 0.0
  []
  [vel_y_aux]
    type = MooseLinearVariableFVReal
    initial_condition = 0.0
  []
  [pressure_aux]
    type = MooseLinearVariableFVReal
    initial_condition = 0.0
  []
[]

[AuxKernels]
  [vel_x_function_aux]
    type = FunctionAux
    variable = vel_x_aux
    function = exact_u
    execute_on = TIMESTEP_END
  []
  [vel_y_function_aux]
    type = FunctionAux
    variable = vel_y_aux
    function = exact_v
    execute_on = TIMESTEP_END
  []
  [pressure_function_aux]
    type = FunctionAux
    variable = pressure_aux
    function = exact_p
    execute_on = TIMESTEP_END
  []
[]

[Executioner]
  type = Steady
  solve_type = NEWTON
  petsc_options_iname = '-pc_type -ksp_gmres_restart'
  petsc_options_value = 'lu 200'
  line_search = 'none'
  nl_abs_tol = 1e-12
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    execute_on = 'timestep_end'
  []
  [L2u]
    type = ElementL2Error
    variable = vel_x
    function = exact_u
    execute_on = 'timestep_end'
  []
  [L2v]
    type = ElementL2Error
    variable = vel_y
    function = exact_v
    execute_on = 'timestep_end'
  []
  [L2p]
    type = ElementL2Error
    variable = pressure
    function = exact_p
    execute_on = 'timestep_end'
  []
[]

[Outputs]
  csv = true
  exodus = true
  execute_on = timestep_end
[]
