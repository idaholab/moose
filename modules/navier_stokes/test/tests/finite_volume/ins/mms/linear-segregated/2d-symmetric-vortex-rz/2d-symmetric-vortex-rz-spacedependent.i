mu_ref = 1.2
mu_r = 0.1
mu_x = 0.2
rho_ref = 1.4
rho_r = 0.15
rho_x = 0.25

use_dev = true

advected_interp_method = 'average'

[Problem]
  linear_sys_names = 'u_system v_system pressure_system'
  previous_nl_solution_required = true
[]

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 2
    ny = 2
  []
  coord_type = 'RZ'
  rz_coord_axis = x
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = rho_fun
    p_diffusion_kernel = p_diffusion
    pressure_projection_method = consistent
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    initial_condition = 0.0
    solver_sys = u_system
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0.0
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
    initial_condition = 0.0
  []
[]

[LinearFVKernels]
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method = ${advected_interp_method}
    mu = mu_fun
    u = vel_x
    v = vel_y
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
    use_deviatoric_terms = ${use_dev}
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method = ${advected_interp_method}
    mu = mu_fun
    u = vel_x
    v = vel_y
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
    use_deviatoric_terms = ${use_dev}
  []
  [u_pressure]
    type = LinearFVMomentumPressure
    variable = vel_x
    pressure = pressure
    momentum_component = 'x'
  []
  [v_pressure]
    type = LinearFVMomentumPressure
    variable = vel_y
    pressure = pressure
    momentum_component = 'y'
  []
  [u_forcing]
    type = LinearFVSource
    variable = vel_x
    source_density = forcing_u
  []
  [v_forcing]
    type = LinearFVSource
    variable = vel_y
    source_density = forcing_v
  []
  [v_viscous_forcing]
    type = LinearFVRZViscousSource
    variable = vel_y
    mu = mu_fun
    momentum_component = 'y'
    u = vel_x
    v = vel_y
    use_deviatoric_terms = ${use_dev}
  []
  [p_diffusion]
    type = LinearFVAnisotropicDiffusion
    variable = pressure
    diffusion_tensor = 'Ainv'
    use_nonorthogonal_correction = false
    use_nonorthogonal_correction_on_boundary = false
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = 'HbyA'
    force_boundary_execution = true
  []
[]

[LinearFVBCs]
  [no-slip-wall-u]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top'
    variable = vel_x
    functor = exact_u
  []
  [no-slip-wall-v]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top'
    variable = vel_y
    functor = exact_v
  []
  [symmetry-u]
    type = LinearFVVelocitySymmetryBC
    boundary = 'bottom'
    variable = vel_x
    u = vel_x
    v = vel_y
    momentum_component = x
  []
  [symmetry-v]
    type = LinearFVVelocitySymmetryBC
    boundary = 'bottom'
    variable = vel_y
    u = vel_x
    v = vel_y
    momentum_component = y
  []

  [pressure-extrapolation]
    type = LinearFVExtrapolatedPressureBC
    boundary = 'top left right'
    variable = pressure
    use_two_term_expansion = true
  []
  [pressure-symmetry]
    type = LinearFVPressureSymmetryBC
    boundary = 'bottom'
    variable = pressure
    HbyA_flux = 'HbyA'
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
  [Q]
    type=ParsedFunction
    expression='4*y^2 - 10*y^3 + 6*y^4'
  []
  [Qp]
    type=ParsedFunction
    expression='8*y - 30*y^2 + 24*y^3'
  []
  [Qpp]
    type=ParsedFunction
    expression='8 - 60*y + 72*y^2'
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
  # --------------------------------------------------
  # Exact solutions
  # --------------------------------------------------
  [exact_u]
    type=ParsedFunction
    symbol_names = 'A Q rho_fun'
    symbol_values = 'A Q rho_fun'
    expression='A*Q / rho_fun'
  []
  [exact_v]
    type=ParsedFunction
    symbol_names = 'Ap R rho_fun'
    symbol_values = 'Ap R rho_fun'
    expression='-Ap*R / rho_fun'
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
    symbol_names = 'A Ap Q rho_fun rho_x'
    symbol_values = 'A Ap Q rho_fun ${rho_x}'
    expression='(Ap*Q)/rho_fun - (A*Q*rho_x)/rho_fun^2'
  []
  [ur]
    type=ParsedFunction
    symbol_names = 'A Q Qp rho_fun rho_r'
    symbol_values = 'A Q Qp rho_fun ${rho_r}'
    expression='(A*Qp)/rho_fun - (A*Q*rho_r)/rho_fun^2'
  []
  [vx]
    type=ParsedFunction
    symbol_names = 'App Ap R rho_fun rho_x'
    symbol_values = 'App Ap R rho_fun ${rho_x}'
    expression='-(App*R)/rho_fun + (Ap*R*rho_x)/rho_fun^2'
  []
  [vr]
    type=ParsedFunction
    symbol_names = 'Ap Rp R rho_fun rho_r'
    symbol_values = 'Ap Rp R rho_fun ${rho_r}'
    expression='-(Ap*Rp)/rho_fun + (Ap*R*rho_r)/rho_fun^2'
  []
  # --------------------------------------------------
  # Second derivatives
  # --------------------------------------------------
  [uxx]
    type = ParsedFunction
    symbol_names  = 'rho_x App Q S Ap S2 A S3'
    symbol_values = '${rho_x} App Q S Ap S2 A S3'
    expression = 'App*Q*S - 2*Ap*Q*rho_x*S2 + 2*A*Q*(rho_x^2)*S3'
  []
  [uxr]
    type = ParsedFunction
    symbol_names  = 'rho_x rho_r Ap Qp S Q S2 A S3'
    symbol_values = '${rho_x} ${rho_r} Ap Qp S Q S2 A S3'
    expression = 'Ap*Qp*S - Ap*Q*rho_r*S2 - A*Qp*rho_x*S2 + 2*A*Q*(rho_x*rho_r)*S3'
  []
  [urr]
    type = ParsedFunction
    symbol_names  = 'rho_r A Qpp S Qp S2 Q S3'
    symbol_values = '${rho_r} A Qpp S Qp S2 Q S3'
    expression = 'A*Qpp*S - 2*A*Qp*rho_r*S2 + 2*A*Q*(rho_r^2)*S3'
  []
  [vxx]
    type = ParsedFunction
    symbol_names  = 'rho_x Appp R S App S2 Ap S3'
    symbol_values = '${rho_x} Appp R S App S2 Ap S3'
    expression = '-Appp*R*S + 2*App*R*rho_x*S2 - 2*Ap*R*(rho_x^2)*S3'
  []
  [vxr]
    type = ParsedFunction
    symbol_names  = 'rho_x rho_r App Rp S R Ap S2 S3'
    symbol_values = '${rho_x} ${rho_r} App Rp S R Ap S2 S3'
    expression = '-App*Rp*S + (App*R*rho_r + Ap*Rp*rho_x)*S2 - 2*Ap*R*(rho_x*rho_r)*S3'
  []
  [vrr]
    type = ParsedFunction
    symbol_names  = 'rho_r Ap Rpp S Rp S2 R S3'
    symbol_values = '${rho_r} Ap Rpp S Rp S2 R S3'
    expression = '-Ap*Rpp*S + 2*Ap*Rp*rho_r*S2 - 2*Ap*R*(rho_r^2)*S3'
  []
  # --------------------------------------------------
  # Derivatives of theta
  # --------------------------------------------------
  [theta]
    type = ParsedFunction
    symbol_names = 'ux vr exact_v'
    symbol_values = 'ux vr exact_v'
    expression = 'ux + vr + (exact_v/y)'
  []
  [thetax]
    type = ParsedFunction
    symbol_names = 'uxx vxr vx'
    symbol_values = 'uxx vxr vx'
    expression = 'uxx + vxr + (vx/y)'
  []
  [thetar]
    type = ParsedFunction
    symbol_names = 'uxr vrr vr exact_v'
    symbol_values = 'uxr vrr vr exact_v'
    expression = 'uxr + vrr + (vr/y) - (exact_v/(y^2))'
  []
  # --------------------------------------------------
  # Stress components
  # --------------------------------------------------
  [tau_xx]
    type=ParsedFunction
    symbol_names = 'ux theta mu_fun'
    symbol_values = 'ux theta mu_fun'
    expression='2*mu_fun*ux - (2.0/3.0)*mu_fun*theta'
  []
  [tau_rr]
    type=ParsedFunction
    symbol_names = 'vr theta mu_fun'
    symbol_values = 'vr theta mu_fun'
    expression='2*mu_fun*vr - (2.0/3.0)*mu_fun*theta'
  []
  [tau_xr]
    type=ParsedFunction
    symbol_names = 'ur vx mu_fun'
    symbol_values = 'ur vx mu_fun'
    expression='mu_fun*(ur + vx)'
  []
  [tau_tt]
    type=ParsedFunction
    symbol_names = 'exact_v theta mu_fun'
    symbol_values = 'exact_v theta mu_fun'
    expression='2*mu_fun*(exact_v/y) - (2.0/3.0)*mu_fun*theta'
  []
  # --------------------------------------------------
  # Convective fluxes
  # --------------------------------------------------
  [Fxx]
    type=ParsedFunction
    symbol_names = 'exact_u rho_fun'
    symbol_values = 'exact_u rho_fun'
    expression='rho_fun*exact_u*exact_u'
  []
  [Fxr]
    type=ParsedFunction
    symbol_names = 'exact_u exact_v rho_fun'
    symbol_values = 'exact_u exact_v rho_fun'
    expression='rho_fun*exact_u*exact_v'
  []
  [Frx]
    type=ParsedFunction
    symbol_names = 'exact_u exact_v rho_fun'
    symbol_values = 'exact_u exact_v rho_fun'
    expression='rho_fun*exact_v*exact_u'
  []
  [Frr]
    type=ParsedFunction
    symbol_names = 'exact_v rho_fun'
    symbol_values = 'exact_v rho_fun'
    expression='rho_fun*exact_v*exact_v'
  []
  # --------------------------------------------------
  # Stress derivatives
  # --------------------------------------------------
  [tau_xx_x]
    type = ParsedFunction
    symbol_names  = 'mu_x ux mu_fun uxx theta thetax'
    symbol_values = '${mu_x} ux mu_fun uxx theta thetax'
    expression = '2*mu_x*ux + 2*mu_fun*uxx - (2.0/3.0)*(mu_x*theta + mu_fun*thetax)'
  []
  [tau_xr_x]
    type = ParsedFunction
    symbol_names  = 'mu_x ur vx mu_fun uxr vxx'
    symbol_values = '${mu_x} ur vx mu_fun uxr vxx'
    expression = 'mu_x*(ur + vx) + mu_fun*(uxr + vxx)'
  []
  [tau_xr_r]
    type = ParsedFunction
    symbol_names  = 'mu_r ur vx mu_fun urr vxr'
    symbol_values = '${mu_r} ur vx mu_fun urr vxr'
    expression = 'mu_r*(ur + vx) + mu_fun*(urr + vxr)'
  []
  [tau_rr_r]
    type = ParsedFunction
    symbol_names  = 'mu_r vr mu_fun vrr theta thetar'
    symbol_values = '${mu_r} vr mu_fun vrr theta thetar'
    expression = '2*mu_r*vr + 2*mu_fun*vrr - (2.0/3.0)*(mu_r*theta + mu_fun*thetar)'
  []
  # --------------------------------------------------
  # Conservative flux derivatives
  # --------------------------------------------------
  [Fxx_x]
    type = ParsedFunction
    symbol_names  = 'rho_x A Ap Q S S2'
    symbol_values = '${rho_x} A Ap Q S S2'
    expression = '2*A*Ap*Q^2*S - A^2*Q^2*rho_x*S2'
  []
  [Frx_r]
    type = ParsedFunction
    symbol_names  = 'rho_r A Ap Qp R Q Rp S S2'
    symbol_values = '${rho_r} A Ap Qp R Q Rp S S2'
    expression = '-A*Ap*((Qp*R + Q*Rp)*S - Q*R*rho_r*S2)'
  []
  [Fxr_x]
    type = ParsedFunction
    symbol_names  = 'rho_x Ap A App Q R S S2'
    symbol_values = '${rho_x} Ap A App Q R S S2'
    expression = '-((Ap^2 + A*App)*Q*R*S - A*Ap*Q*R*rho_x*S2)'
  []
  [Frr_r]
    type = ParsedFunction
    symbol_names  = 'rho_r Ap R Rp S S2'
    symbol_values = '${rho_r} Ap R Rp S S2'
    expression = 'Ap^2*(2*R*Rp*S - R^2*rho_r*S2)'
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
  # Forcing terms
  # --------------------------------------------------
  [forcing_u]
    type = ParsedFunction
    symbol_names  = 'Fxx_x Frx Frx_r px tau_xx_x tau_xr tau_xr_r'
    symbol_values = 'Fxx_x Frx Frx_r px tau_xx_x tau_xr tau_xr_r'
    expression = 'Fxx_x + (Frx/y) + Frx_r + px - ( tau_xx_x + (tau_xr/y) + tau_xr_r )'
  []
  [forcing_v]
    type = ParsedFunction
    symbol_names  = 'Fxr_x Frr Frr_r pr tau_xr_x tau_rr tau_rr_r tau_tt'
    symbol_values = 'Fxr_x Frr Frr_r pr tau_xr_x tau_rr tau_rr_r tau_tt'
    expression = 'Fxr_x + (Frr/y) + Frr_r + pr - ( tau_xr_x + (tau_rr/y) + tau_rr_r - (tau_tt)/y )'
  []
[]

[Executioner]
  type = SIMPLE
  momentum_l_abs_tol = 1e-10
  pressure_l_abs_tol = 1e-10
  momentum_l_tol = 0
  pressure_l_tol = 0
  momentum_l_max_its = 500
  pressure_l_max_its = 500
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  momentum_equation_relaxation = 0.85
  pressure_variable_relaxation = 1.0
  num_iterations = 10000
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-8
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  continue_on_max_its = true

  pin_pressure = true
  pressure_pin_value = 0.125
  pressure_pin_point = '0.5 0.5 0.0'
[]

[Outputs]
  [csv]
    type = CSV
    execute_on = TIMESTEP_END
  []
[]

[Postprocessors]
  [h]
    type = AverageElementSize
    execute_on = TIMESTEP_END
  []
  [L2u]
    type = ElementL2FunctorError
    approximate = vel_x
    exact = exact_u
    execute_on = TIMESTEP_END
  []
  [L2v]
    type = ElementL2FunctorError
    approximate = vel_y
    exact = exact_v
    execute_on = TIMESTEP_END
  []
  [L2p]
    approximate = pressure
    exact = exact_p
    type = ElementL2FunctorError
    execute_on = TIMESTEP_END
  []
[]
