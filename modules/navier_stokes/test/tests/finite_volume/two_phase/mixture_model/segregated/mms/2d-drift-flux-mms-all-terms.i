# Method of manufactured solutions for the linear finite volume two phase drift flux model with
# EVERY term of the four equation system active simultaneously.
#
# The three studies alongside this one each hold most of the model still: the spatial and temporal
# ones fix every density, and the varying-density one moves rho_d in the phase equation only. None
# of them carries an energy equation, an interfacial mass transfer, a mixture density transient in
# the pressure equation, or a continuous phase density that moves. This case carries all of them at
# once, so that no term can be verified by a study in which it is absent and no pair of terms can
# cancel each other's error unobserved.
#
# Active here, and absent from at least one of the other studies:
#   mass      d(rho_m)/dt in the pressure equation, at varying rho_d AND rho_c
#   momentum  conservative transient at varying rho_m, advection, viscous stress, pressure,
#             the diffusion stress beta_d beta_c / rho_m u_s (x) u_s, gravity
#   phase     conservative transient at varying rho_d, the alpha d(rho_d)/dt product rule half,
#             drift advection, phase diffusion, interfacial mass transfer Gamma
#   energy    conservative transient at varying rho_m c_pm, advection, the cross term
#             beta_d beta_c / rho_m (cp_d - cp_c) T u_s, conduction, latent heat Gamma h_lat
#
# Not active: the interfacial area equation. Its coalescence and breakage closures carry fractional
# powers of an energy dissipation rate that is itself a closure of the flow, so a manufactured
# forcing for them would be a transcription of the source terms rather than an independent
# statement of them. It is verified separately against its own steady states.
#
# The forcing functions were derived symbolically by derive-all-terms-forcings.py and then checked
# against a finite difference evaluation of the same residuals, built independently from the
# manufactured solution alone; they agree to 1e-7, which is the finite difference truncation.
#
# TWO SETTINGS THAT LOOK ARBITRARY AND ARE NOT.
#
# k_c and k_d. The cell Peclet number of the ENERGY equation, rho_m cp_m |u| h / k_m, has to stay
# well below one or the central interpolation is under-resolved and the measured order is depressed
# long before the asymptotic regime is reached. At the k_c = 0.5, k_d = 0.2 that a physical
# air-water case would use, Pe is about 1.8 at n = 40 and the temperature converges at 1.43 while
# every other field converges at 2. It is not a defect: raising k to 5, 50, 500 walks the order back
# to 1.98, 2.04, 2.09. The values here put Pe near 0.1. The phase equation never had the problem
# because its own Peclet number is ten times smaller.
#
# pressure_absolute_tolerance. Absolute residual norms grow with the mesh. At 1e-11 the pressure
# equation stalls at its round-off floor, about 3e-10 at n = 160, the solve is declared failed and
# the time stepper cuts dt until nothing progresses. 1e-8 is reachable at 320x320 and still two
# orders below the discretisation error there.
#
# Manufactured solution:
#   u   =  x^2 (1-x)^2 (4y^3 - 6y^2 + 2y) (1 + sin(2 pi t)/2)
#   v   = -y^2 (1-y)^2 (4x^3 - 6x^2 + 2x) (1 + sin(2 pi t)/2)
#   p   =  x (1-x) (1 + sin(2 pi t)/2)
#   phi =  1/2 + sin(2 pi t) sin(pi x) sin(pi y)/5
#   T   =  1 + sin(pi x) sin(pi y) sin(2 pi t)/2
#   rho_d = rho_d0 (1 + sin(2 pi t)/2)      rho_c = rho_c0 (1 + sin(2 pi t)/4)
# The phase fraction stays inside [0.3, 0.7], away from the property limiter. The velocity is NOT
# divergence free once rho_m moves, which is the point: the mass equation carries a real forcing.
#
# The slip is a prescribed constant vector, as in the other studies: this verifies the
# discretisation of the terms the slip enters, not the algebraic closure that produces it. The
# closure is verified separately against its analytic value.

mu_c = 1.0
mu_d = 0.5
rho_c0 = 1000.0
rho_d0 = 400.0
cp_c = 2.0
cp_d = 1.2
k_c = 10.0
k_d = 4.0
a = 0.04
b = 0.03
Dphi = 0.01
gx = -0.8
gy = 0.3
hlat = 5.0
G0 = 3.0
advected_interp_method = 'average'

[Problem]
  linear_sys_names = 'u_system v_system pressure_system phi_system energy_system'
  previous_nl_solution_required = true
[]

# 1 when the energy equation carries the pressure work. The discrete term is then assembled from
# the same functor the Physics builds, and the manufactured value is subtracted, so what is left to
# balance is the discretisation error of the term itself. 0 leaves the study as it was.
pw_on = 0

[Mesh]
  [gmg]
    type = GeneratedMeshGenerator
    dim = 2
    nx = 20
    ny = 20
  []
[]

[UserObjects]
  [rc]
    type = RhieChowMassFlux
    u = vel_x
    v = vel_y
    pressure = pressure
    rho = rho_mixture
    p_diffusion_kernel = p_diffusion
    pressure_projection_method = CONSISTENT
  []
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
  []
  [pressure]
    type = MooseLinearVariableFVReal
    solver_sys = pressure_system
  []
  [phase_2]
    type = MooseLinearVariableFVReal
    solver_sys = phi_system
  []
  [T_fluid]
    type = MooseLinearVariableFVReal
    solver_sys = energy_system
  []
[]

[ICs]
  [u_ic]
    type = FunctionIC
    variable = vel_x
    function = exact_u
  []
  [v_ic]
    type = FunctionIC
    variable = vel_y
    function = exact_v
  []
  [p_ic]
    type = FunctionIC
    variable = pressure
    function = exact_p
  []
  [phi_ic]
    type = FunctionIC
    variable = phase_2
    function = exact_phi
  []
  [T_ic]
    type = FunctionIC
    variable = T_fluid
    function = exact_T
  []
[]

[FVInterpolationMethods]
  [average]
    type = FVGeometricAverage
  []
  [upwind]
    type = FVAdvectedUpwind
  []
[]

[FunctorMaterials]
  [phase_dot]
    type = GenericFunctorTimeDerivativeMaterial
    prop_names = 'dphi_dt'
    prop_values = 'phase_2'
  []
  [mixture_props]
    type = ParsedFunctorMaterial
    property_name = 'rho_mixture'
    expression = 'phase_2 * rho_d_t + (1 - phase_2) * rho_c_t'
    functor_names = 'phase_2 rho_d_t rho_c_t'
  []
  [mu_mixture]
    type = ParsedFunctorMaterial
    property_name = 'mu_mixture'
    expression = 'phase_2 * ${mu_d} + (1 - phase_2) * ${mu_c}'
    functor_names = 'phase_2'
  []
  [k_mixture]
    type = ParsedFunctorMaterial
    property_name = 'k_mixture'
    expression = 'phase_2 * ${k_d} + (1 - phase_2) * ${k_c}'
    functor_names = 'phase_2'
  []
  # mass weighted, as the energy split requires
  [cp_mixture]
    type = ParsedFunctorMaterial
    property_name = 'cp_mixture'
    expression = '(phase_2 * rho_d_t * ${cp_d} + (1 - phase_2) * rho_c_t * ${cp_c}) / rho_mixture'
    functor_names = 'phase_2 rho_d_t rho_c_t rho_mixture'
  []
  [rho_cp]
    type = ParsedFunctorMaterial
    property_name = 'rho_cp'
    expression = 'phase_2 * rho_d_t * ${cp_d} + (1 - phase_2) * rho_c_t * ${cp_c}'
    functor_names = 'phase_2 rho_d_t rho_c_t'
  []
  # d(rho_m)/dt = (rho_d - rho_c) d(alpha)/dt + alpha d(rho_d)/dt + (1 - alpha) d(rho_c)/dt
  [drho_m_dt]
    type = ParsedFunctorMaterial
    property_name = 'drho_m_dt'
    expression = '(rho_d_t - rho_c_t) * dphi_dt + phase_2 * drho_d_dt + (1 - phase_2) * drho_c_dt'
    functor_names = 'rho_d_t rho_c_t dphi_dt phase_2 drho_d_dt drho_c_dt'
  []
  [rho_d_D]
    type = ParsedFunctorMaterial
    property_name = 'rho_d_D'
    expression = 'rho_d_t * ${Dphi}'
    functor_names = 'rho_d_t'
  []
  [gravity_x]
    type = ParsedFunctorMaterial
    property_name = 'rho_g_x'
    expression = 'rho_mixture * ${gx}'
    functor_names = 'rho_mixture'
  []
  [gravity_y]
    type = ParsedFunctorMaterial
    property_name = 'rho_g_y'
    expression = 'rho_mixture * ${gy}'
    functor_names = 'rho_mixture'
  []
  [gamma_hlat]
    type = ParsedFunctorMaterial
    property_name = 'gamma_hlat'
    expression = 'gamma_fn * ${hlat}'
    functor_names = 'gamma_fn'
  []
  [pw_velocity_x]
    type = ParsedFunctorMaterial
    property_name = 'pw_velocity_x'
    expression = 'vel_x + (phase_2 - phase_2 * rho_d_t / rho_mixture) * ${a}'
    functor_names = 'vel_x phase_2 rho_d_t rho_mixture'
  []
  [pw_velocity_y]
    type = ParsedFunctorMaterial
    property_name = 'pw_velocity_y'
    expression = 'vel_y + (phase_2 - phase_2 * rho_d_t / rho_mixture) * ${b}'
    functor_names = 'vel_y phase_2 rho_d_t rho_mixture'
  []
  [pressure_work]
    type = NSFVPressureWorkFunctorMaterial
    pressure = pressure
    u = pw_velocity_x
    v = pw_velocity_y
    pressure_work_name = 'pressure_work'
  []
[]

[LinearFVKernels]
  # ---------------- momentum ----------------
  [u_time]
    type = LinearFVTimeDerivative
    variable = vel_x
    factor = rho_mixture
  []
  [v_time]
    type = LinearFVTimeDerivative
    variable = vel_y
    factor = rho_mixture
  []
  [u_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_x
    advected_interp_method_name = ${advected_interp_method}
    mu = mu_mixture
    u = vel_x
    v = vel_y
    momentum_component = 'x'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
  []
  [v_advection_stress]
    type = LinearWCNSFVMomentumFlux
    variable = vel_y
    advected_interp_method_name = ${advected_interp_method}
    mu = mu_mixture
    u = vel_x
    v = vel_y
    momentum_component = 'y'
    rhie_chow_user_object = 'rc'
    use_nonorthogonal_correction = false
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
  [u_drift_flux]
    type = LinearWCNSFV2PMomentumDriftFlux
    variable = vel_x
    momentum_component = 'x'
    u_slip = ${a}
    v_slip = ${b}
    rho_d = rho_d_t
    rho_c = rho_c_t
    fraction_dispersed = phase_2
    density_interp_method = 'average'
    rhie_chow_user_object = 'rc'
  []
  [v_drift_flux]
    type = LinearWCNSFV2PMomentumDriftFlux
    variable = vel_y
    momentum_component = 'y'
    u_slip = ${a}
    v_slip = ${b}
    rho_d = rho_d_t
    rho_c = rho_c_t
    fraction_dispersed = phase_2
    density_interp_method = 'average'
    rhie_chow_user_object = 'rc'
  []
  [u_gravity]
    type = LinearFVSource
    variable = vel_x
    source_density = rho_g_x
  []
  [v_gravity]
    type = LinearFVSource
    variable = vel_y
    source_density = rho_g_y
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

  # ---------------- pressure / mixture mass ----------------
  [p_diffusion]
    type = LinearFVPressureCorrectionDiffusion
    variable = pressure
    diffusion_tensor = Ainv
    use_nonorthogonal_correction = false
    use_nonorthogonal_correction_on_boundary = false
  []
  [HbyA_divergence]
    type = LinearFVDivergence
    variable = pressure
    face_flux = HbyA
    force_boundary_execution = true
  []
  # the storage term of mixture continuity, the same sign the Physics uses
  [p_density_transient]
    type = LinearFVSource
    variable = pressure
    source_density = drho_m_dt
    scaling_factor = -1
  []
  # the manufactured mass source, opposite sign because it sits on the other side
  [p_forcing]
    type = LinearFVSource
    variable = pressure
    source_density = forcing_mass
    scaling_factor = 1
  []

  # ---------------- dispersed phase ----------------
  [phase_time]
    type = LinearFVTimeDerivative
    variable = phase_2
    factor = rho_d_t
  []
  [phase_advection]
    type = LinearFVScalarAdvection
    variable = phase_2
    advected_interp_method_name = ${advected_interp_method}
    rhie_chow_user_object = 'rc'
    density = rho_d_t
    u_slip = ${a}
    v_slip = ${b}
    slip_boundaries = 'left right top bottom'
  []
  [phase_diffusion]
    type = LinearFVDiffusion
    variable = phase_2
    diffusion_coeff = rho_d_D
    use_nonorthogonal_correction = false
  []
  [phase_gamma]
    type = LinearFVSource
    variable = phase_2
    source_density = gamma_fn
    scaling_factor = 1
  []
  [phase_forcing]
    type = LinearFVSource
    variable = phase_2
    source_density = forcing_phi
  []

  # ---------------- energy ----------------
  [T_time]
    type = LinearFVTimeDerivative
    variable = T_fluid
    factor = rho_cp
  []
  [T_advection]
    type = LinearFVEnergyAdvection
    variable = T_fluid
    advected_quantity = 'temperature'
    cp = cp_mixture
    rhie_chow_user_object = 'rc'
    advected_interp_method = 'average'
  []
  [T_drift_flux]
    type = LinearWCNSFV2PEnergyDriftFlux
    variable = T_fluid
    rho_d = rho_d_t
    rho_c = rho_c_t
    cp_d = ${cp_d}
    cp_c = ${cp_c}
    fraction_dispersed = phase_2
    advected_interp_method = 'average'
    slip_boundaries = 'left right top bottom'
    u_slip = ${a}
    v_slip = ${b}
  []
  [T_diffusion]
    type = LinearFVDiffusion
    variable = T_fluid
    diffusion_coeff = k_mixture
    use_nonorthogonal_correction = false
  []
  [T_latent]
    type = LinearFVSource
    variable = T_fluid
    source_density = gamma_hlat
    scaling_factor = -1
  []
  [T_forcing]
    type = LinearFVSource
    variable = T_fluid
    source_density = forcing_T
  []
  [T_pressure_work_mms]
    type = LinearFVSource
    variable = T_fluid
    source_density = mms_pressure_work
    scaling_factor = ${fparse -pw_on}
  []
  [T_pressure_work]
    type = LinearFVSource
    variable = T_fluid
    source_density = pressure_work
    scaling_factor = ${pw_on}
  []
[]

[LinearFVBCs]
  [u_all]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top bottom'
    variable = vel_x
    functor = exact_u
  []
  [v_all]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top bottom'
    variable = vel_y
    functor = exact_v
  []
  # The pressure needs one Dirichlet boundary. With an extrapolated condition on all four the
  # pressure system is singular and the field is fixed only up to a constant. The pin does not
  # rescue it: with a pin value of zero the constraint adds nothing to the right hand side and only
  # doubles a diagonal, which fixes the constant solely when the discrete system is compatible and
  # fails silently when it is not. Without this the study reports a pressure wrong by three orders
  # of magnitude while every other field still converges, because nothing else reads the pressure
  # itself, only its gradient.
  [pressure_dirichlet]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left'
    variable = pressure
    functor = exact_p
  []
  [pressure_extrapolation]
    type = LinearFVExtrapolatedPressureBC
    boundary = 'right top bottom'
    variable = pressure
    use_two_term_expansion = true
  []
  [phase_all]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top bottom'
    variable = phase_2
    functor = exact_phi
  []
  [T_all]
    type = LinearFVAdvectionDiffusionFunctorDirichletBC
    boundary = 'left right top bottom'
    variable = T_fluid
    functor = exact_T
  []
[]

[Functions]
  [exact_u]
    type = ParsedFunction
    expression = 'x^2*(1 - x)^2*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y)'
  []
  [exact_v]
    type = ParsedFunction
    expression = '-y^2*(1 - y)^2*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x)'
  []
  [exact_p]
    type = ParsedFunction
    expression = '(x*(1 - x) - 1/4)*(sin(2*pi*t)/2 + 1)'
  []
  [exact_phi]
    type = ParsedFunction
    expression = 'sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2'
  []
  [exact_T]
    type = ParsedFunction
    expression = 'sin(2*pi*t)*sin(pi*x)*sin(pi*y)/2 + 1'
  []
  [rho_d_t]
    type = ParsedFunction
    expression = 'rho_d0*(sin(2*pi*t)/2 + 1)'
    symbol_names = 'mu_c mu_d rho_c0 rho_d0 cp_c cp_d k_c k_d a b Dphi gx gy hlat G0'
    symbol_values = '${mu_c} ${mu_d} ${rho_c0} ${rho_d0} ${cp_c} ${cp_d} ${k_c} ${k_d} ${a} ${b} ${Dphi} ${gx} ${gy} ${hlat} ${G0}'
  []
  [rho_c_t]
    type = ParsedFunction
    expression = 'rho_c0*(sin(2*pi*t)/4 + 1)'
    symbol_names = 'mu_c mu_d rho_c0 rho_d0 cp_c cp_d k_c k_d a b Dphi gx gy hlat G0'
    symbol_values = '${mu_c} ${mu_d} ${rho_c0} ${rho_d0} ${cp_c} ${cp_d} ${k_c} ${k_d} ${a} ${b} ${Dphi} ${gx} ${gy} ${hlat} ${G0}'
  []
  [drho_d_dt]
    type = ParsedFunction
    expression = 'pi*rho_d0*cos(2*pi*t)'
    symbol_names = 'mu_c mu_d rho_c0 rho_d0 cp_c cp_d k_c k_d a b Dphi gx gy hlat G0'
    symbol_values = '${mu_c} ${mu_d} ${rho_c0} ${rho_d0} ${cp_c} ${cp_d} ${k_c} ${k_d} ${a} ${b} ${Dphi} ${gx} ${gy} ${hlat} ${G0}'
  []
  [drho_c_dt]
    type = ParsedFunction
    expression = 'pi*rho_c0*cos(2*pi*t)/2'
    symbol_names = 'mu_c mu_d rho_c0 rho_d0 cp_c cp_d k_c k_d a b Dphi gx gy hlat G0'
    symbol_values = '${mu_c} ${mu_d} ${rho_c0} ${rho_d0} ${cp_c} ${cp_d} ${k_c} ${k_d} ${a} ${b} ${Dphi} ${gx} ${gy} ${hlat} ${G0}'
  []
  [gamma_fn]
    type = ParsedFunction
    expression = 'G0*(sin(2*pi*t) + 1)*sin(pi*x)*sin(pi*y)/2'
    symbol_names = 'mu_c mu_d rho_c0 rho_d0 cp_c cp_d k_c k_d a b Dphi gx gy hlat G0'
    symbol_values = '${mu_c} ${mu_d} ${rho_c0} ${rho_d0} ${cp_c} ${cp_d} ${k_c} ${k_d} ${a} ${b} ${Dphi} ${gx} ${gy} ${hlat} ${G0}'
  []
  [mms_pressure_work]
    type = ParsedFunction
    expression = '(1 - 2*x)*(a*(-rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1)/(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1)) + sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2) + x^2*(1 - x)^2*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y))*(sin(2*pi*t)/2 + 1) + pi*(x*(1 - x) - 1/4)*cos(2*pi*t)'
    symbol_names = 'mu_c mu_d rho_c0 rho_d0 cp_c cp_d k_c k_d a b Dphi gx gy hlat G0'
    symbol_values = '${mu_c} ${mu_d} ${rho_c0} ${rho_d0} ${cp_c} ${cp_d} ${k_c} ${k_d} ${a} ${b} ${Dphi} ${gx} ${gy} ${hlat} ${G0}'
  []
  [forcing_mass]
    type = ParsedFunction
    expression = 'pi*rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*cos(2*pi*t)/2 - 2*pi*rho_c0*(sin(2*pi*t)/4 + 1)*sin(pi*x)*sin(pi*y)*cos(2*pi*t)/5 + pi*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*cos(2*pi*t) + 2*pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(pi*x)*sin(pi*y)*cos(2*pi*t)/5 + x^2*(1 - x)^2*(-pi*rho_c0*(sin(2*pi*t)/4 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5 + pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5)*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y) + x^2*(2*x - 2)*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y) + 2*x*(1 - x)^2*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y) - y^2*(1 - y)^2*(-pi*rho_c0*(sin(2*pi*t)/4 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5 + pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5)*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x) - y^2*(2*y - 2)*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x) - 2*y*(1 - y)^2*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x)'
    symbol_names = 'mu_c mu_d rho_c0 rho_d0 cp_c cp_d k_c k_d a b Dphi gx gy hlat G0'
    symbol_values = '${mu_c} ${mu_d} ${rho_c0} ${rho_d0} ${cp_c} ${cp_d} ${k_c} ${k_d} ${a} ${b} ${Dphi} ${gx} ${gy} ${hlat} ${G0}'
  []
  [forcing_u]
    type = ParsedFunction
    expression = 'pi*a^2*rho_c0*rho_d0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/(5*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))) - pi*a^2*rho_c0*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/(5*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))) + a^2*rho_c0*rho_d0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(pi*rho_c0*(sin(2*pi*t)/4 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5 - pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)/(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))^2 + pi*a*b*rho_c0*rho_d0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/(5*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))) - pi*a*b*rho_c0*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/(5*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))) + a*b*rho_c0*rho_d0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(pi*rho_c0*(sin(2*pi*t)/4 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5 - pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)/(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))^2 - gx*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1)) + x^4*(1 - x)^4*(-pi*rho_c0*(sin(2*pi*t)/4 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5 + pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5)*(sin(2*pi*t)/2 + 1)^2*(4*y^3 - 6*y^2 + 2*y)^2 - 4*x^4*(1 - x)^3*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)^2*(4*y^3 - 6*y^2 + 2*y)^2 + 4*x^3*(1 - x)^4*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)^2*(4*y^3 - 6*y^2 + 2*y)^2 - x^2*y^2*(1 - x)^2*(1 - y)^2*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)^2*(4*x^3 - 6*x^2 + 2*x)*(12*y^2 - 12*y + 2) - x^2*y^2*(1 - x)^2*(1 - y)^2*(-pi*rho_c0*(sin(2*pi*t)/4 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5 + pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5)*(sin(2*pi*t)/2 + 1)^2*(4*x^3 - 6*x^2 + 2*x)*(4*y^3 - 6*y^2 + 2*y) - x^2*y^2*(1 - x)^2*(2*y - 2)*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)^2*(4*x^3 - 6*x^2 + 2*x)*(4*y^3 - 6*y^2 + 2*y) - 2*x^2*y*(1 - x)^2*(1 - y)^2*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)^2*(4*x^3 - 6*x^2 + 2*x)*(4*y^3 - 6*y^2 + 2*y) - x^2*(1 - x)^2*(24*y - 12)*(mu_c*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2) + mu_d*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2))*(sin(2*pi*t)/2 + 1) + pi*x^2*(1 - x)^2*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(4*y^3 - 6*y^2 + 2*y)*cos(2*pi*t) - x^2*(1 - x)^2*(-pi*mu_c*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5 + pi*mu_d*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5)*(sin(2*pi*t)/2 + 1)*(12*y^2 - 12*y + 2) + x^2*(1 - x)^2*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y)*(pi*rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*cos(2*pi*t)/2 - 2*pi*rho_c0*(sin(2*pi*t)/4 + 1)*sin(pi*x)*sin(pi*y)*cos(2*pi*t)/5 + pi*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*cos(2*pi*t) + 2*pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(pi*x)*sin(pi*y)*cos(2*pi*t)/5) - x*(sin(2*pi*t)/2 + 1) + (1 - x)*(sin(2*pi*t)/2 + 1) - (mu_c*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2) + mu_d*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2))*(2*x^2*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y) + 4*x*(2*x - 2)*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y) + 2*(1 - x)^2*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y)) - (x^2*(2*x - 2)*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y) + 2*x*(1 - x)^2*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y))*(-pi*mu_c*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5 + pi*mu_d*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5)'
    symbol_names = 'mu_c mu_d rho_c0 rho_d0 cp_c cp_d k_c k_d a b Dphi gx gy hlat G0'
    symbol_values = '${mu_c} ${mu_d} ${rho_c0} ${rho_d0} ${cp_c} ${cp_d} ${k_c} ${k_d} ${a} ${b} ${Dphi} ${gx} ${gy} ${hlat} ${G0}'
  []
  [forcing_v]
    type = ParsedFunction
    expression = 'pi*a*b*rho_c0*rho_d0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/(5*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))) - pi*a*b*rho_c0*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/(5*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))) + a*b*rho_c0*rho_d0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(pi*rho_c0*(sin(2*pi*t)/4 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5 - pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)/(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))^2 + pi*b^2*rho_c0*rho_d0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/(5*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))) - pi*b^2*rho_c0*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/(5*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))) + b^2*rho_c0*rho_d0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(pi*rho_c0*(sin(2*pi*t)/4 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5 - pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)/(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))^2 - gy*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1)) - x^2*y^2*(1 - x)^2*(1 - y)^2*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)^2*(12*x^2 - 12*x + 2)*(4*y^3 - 6*y^2 + 2*y) - x^2*y^2*(1 - x)^2*(1 - y)^2*(-pi*rho_c0*(sin(2*pi*t)/4 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5 + pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5)*(sin(2*pi*t)/2 + 1)^2*(4*x^3 - 6*x^2 + 2*x)*(4*y^3 - 6*y^2 + 2*y) - x^2*y^2*(1 - y)^2*(2*x - 2)*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)^2*(4*x^3 - 6*x^2 + 2*x)*(4*y^3 - 6*y^2 + 2*y) - 2*x*y^2*(1 - x)^2*(1 - y)^2*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)^2*(4*x^3 - 6*x^2 + 2*x)*(4*y^3 - 6*y^2 + 2*y) + y^4*(1 - y)^4*(-pi*rho_c0*(sin(2*pi*t)/4 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5 + pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5)*(sin(2*pi*t)/2 + 1)^2*(4*x^3 - 6*x^2 + 2*x)^2 - 4*y^4*(1 - y)^3*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)^2*(4*x^3 - 6*x^2 + 2*x)^2 + 4*y^3*(1 - y)^4*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)^2*(4*x^3 - 6*x^2 + 2*x)^2 + y^2*(1 - y)^2*(24*x - 12)*(mu_c*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2) + mu_d*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2))*(sin(2*pi*t)/2 + 1) - pi*y^2*(1 - y)^2*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(4*x^3 - 6*x^2 + 2*x)*cos(2*pi*t) + y^2*(1 - y)^2*(-pi*mu_c*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5 + pi*mu_d*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5)*(sin(2*pi*t)/2 + 1)*(12*x^2 - 12*x + 2) - y^2*(1 - y)^2*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x)*(pi*rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*cos(2*pi*t)/2 - 2*pi*rho_c0*(sin(2*pi*t)/4 + 1)*sin(pi*x)*sin(pi*y)*cos(2*pi*t)/5 + pi*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*cos(2*pi*t) + 2*pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(pi*x)*sin(pi*y)*cos(2*pi*t)/5) - (mu_c*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2) + mu_d*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2))*(-2*y^2*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x) - 4*y*(2*y - 2)*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x) - 2*(1 - y)^2*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x)) - (-y^2*(2*y - 2)*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x) - 2*y*(1 - y)^2*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x))*(-pi*mu_c*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5 + pi*mu_d*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5)'
    symbol_names = 'mu_c mu_d rho_c0 rho_d0 cp_c cp_d k_c k_d a b Dphi gx gy hlat G0'
    symbol_values = '${mu_c} ${mu_d} ${rho_c0} ${rho_d0} ${cp_c} ${cp_d} ${k_c} ${k_d} ${a} ${b} ${Dphi} ${gx} ${gy} ${hlat} ${G0}'
  []
  [forcing_phi]
    type = ParsedFunction
    expression = '2*pi^2*Dphi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 - G0*(sin(2*pi*t) + 1)*sin(pi*x)*sin(pi*y)/2 + pi*rho_d0*(a + x^2*(1 - x)^2*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y))*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5 + pi*rho_d0*(b - y^2*(1 - y)^2*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x))*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5 + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(x^2*(2*x - 2)*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y) + 2*x*(1 - x)^2*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y))*(sin(2*pi*t)/2 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(-y^2*(2*y - 2)*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x) - 2*y*(1 - y)^2*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x))*(sin(2*pi*t)/2 + 1) + pi*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*cos(2*pi*t) + 2*pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(pi*x)*sin(pi*y)*cos(2*pi*t)/5'
    symbol_names = 'mu_c mu_d rho_c0 rho_d0 cp_c cp_d k_c k_d a b Dphi gx gy hlat G0'
    symbol_values = '${mu_c} ${mu_d} ${rho_c0} ${rho_d0} ${cp_c} ${cp_d} ${k_c} ${k_d} ${a} ${b} ${Dphi} ${gx} ${gy} ${hlat} ${G0}'
  []
  [forcing_T]
    type = ParsedFunction
    expression = 'G0*hlat*(sin(2*pi*t) + 1)*sin(pi*x)*sin(pi*y)/2 + pi*a*rho_c0*rho_d0*(-cp_c + cp_d)*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/(2*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))) + pi*a*rho_c0*rho_d0*(-cp_c + cp_d)*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/2 + 1)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/(5*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))) - pi*a*rho_c0*rho_d0*(-cp_c + cp_d)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/2 + 1)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/(5*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))) + a*rho_c0*rho_d0*(-cp_c + cp_d)*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/2 + 1)*(pi*rho_c0*(sin(2*pi*t)/4 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5 - pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)/(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))^2 + pi*b*rho_c0*rho_d0*(-cp_c + cp_d)*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/(2*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))) + pi*b*rho_c0*rho_d0*(-cp_c + cp_d)*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/2 + 1)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/(5*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))) - pi*b*rho_c0*rho_d0*(-cp_c + cp_d)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/2 + 1)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/(5*(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))) + b*rho_c0*rho_d0*(-cp_c + cp_d)*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/2 + 1)*(pi*rho_c0*(sin(2*pi*t)/4 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5 - pi*rho_d0*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5)*(sin(2*pi*t)/4 + 1)*(sin(2*pi*t)/2 + 1)/(rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))^2 + x^2*(1 - x)^2*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/2 + 1)*(-pi*cp_c*rho_c0*(sin(2*pi*t)/4 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5 + pi*cp_d*rho_d0*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5)*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y) + pi*x^2*(1 - x)^2*(cp_c*rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + cp_d*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/2 + x^2*(2*x - 2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/2 + 1)*(cp_c*rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + cp_d*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y) + 2*x*(1 - x)^2*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/2 + 1)*(cp_c*rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + cp_d*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)*(4*y^3 - 6*y^2 + 2*y) - y^2*(1 - y)^2*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/2 + 1)*(-pi*cp_c*rho_c0*(sin(2*pi*t)/4 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5 + pi*cp_d*rho_d0*(sin(2*pi*t)/2 + 1)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5)*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x) - pi*y^2*(1 - y)^2*(cp_c*rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + cp_d*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/2 - y^2*(2*y - 2)*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/2 + 1)*(cp_c*rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + cp_d*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x) - 2*y*(1 - y)^2*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/2 + 1)*(cp_c*rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + cp_d*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*(sin(2*pi*t)/2 + 1)*(4*x^3 - 6*x^2 + 2*x) + pi^2*(k_c*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2) + k_d*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2))*sin(2*pi*t)*sin(pi*x)*sin(pi*y) + (sin(2*pi*t)*sin(pi*x)*sin(pi*y)/2 + 1)*(pi*cp_c*rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*cos(2*pi*t)/2 - 2*pi*cp_c*rho_c0*(sin(2*pi*t)/4 + 1)*sin(pi*x)*sin(pi*y)*cos(2*pi*t)/5 + pi*cp_d*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*cos(2*pi*t) + 2*pi*cp_d*rho_d0*(sin(2*pi*t)/2 + 1)*sin(pi*x)*sin(pi*y)*cos(2*pi*t)/5) + pi*(cp_c*rho_c0*(-sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/4 + 1) + cp_d*rho_d0*(sin(2*pi*t)*sin(pi*x)*sin(pi*y)/5 + 1/2)*(sin(2*pi*t)/2 + 1))*sin(pi*x)*sin(pi*y)*cos(2*pi*t) - pi*(-pi*k_c*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5 + pi*k_d*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/5)*sin(2*pi*t)*sin(pi*x)*cos(pi*y)/2 - pi*(-pi*k_c*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5 + pi*k_d*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/5)*sin(2*pi*t)*sin(pi*y)*cos(pi*x)/2'
    symbol_names = 'mu_c mu_d rho_c0 rho_d0 cp_c cp_d k_c k_d a b Dphi gx gy hlat G0'
    symbol_values = '${mu_c} ${mu_d} ${rho_c0} ${rho_d0} ${cp_c} ${cp_d} ${k_c} ${k_d} ${a} ${b} ${Dphi} ${gx} ${gy} ${hlat} ${G0}'
  []
[]

[Executioner]
  type = PIMPLE
  scheme = bdf2
  end_time = 0.2
  dt = 0.025
  rhie_chow_user_object = 'rc'
  momentum_systems = 'u_system v_system'
  pressure_system = 'pressure_system'
  active_scalar_systems = 'phi_system'
  energy_system = 'energy_system'
  momentum_equation_relaxation = 0.8
  pressure_variable_relaxation = 1.0
  active_scalar_equation_relaxation = '1.0'
  energy_equation_relaxation = 1.0
  num_iterations = 500
  pressure_absolute_tolerance = 1e-8
  momentum_absolute_tolerance = 1e-11
  active_scalar_absolute_tolerance = '1e-11'
  energy_absolute_tolerance = 1e-11
  momentum_l_abs_tol = 1e-13
  pressure_l_abs_tol = 1e-13
  active_scalar_l_abs_tol = 1e-13
  energy_l_abs_tol = 1e-13
  momentum_l_tol = 0
  pressure_l_tol = 0
  active_scalar_l_tol = 0
  energy_l_tol = 0
  momentum_petsc_options_iname = '-pc_type -pc_hypre_type'
  momentum_petsc_options_value = 'hypre boomeramg'
  pressure_petsc_options_iname = '-pc_type -pc_hypre_type'
  pressure_petsc_options_value = 'hypre boomeramg'
  active_scalar_petsc_options_iname = '-pc_type -pc_hypre_type'
  active_scalar_petsc_options_value = 'hypre boomeramg'
  energy_petsc_options_iname = '-pc_type -pc_hypre_type'
  energy_petsc_options_value = 'hypre boomeramg'
  print_fields = false
  pin_pressure = false
[]

[AuxVariables]
  [pw_sampled]
    type = MooseVariableFVReal
  []
[]

[AuxKernels]
  [sample_pressure_work]
    type = FunctorAux
    variable = pw_sampled
    functor = pressure_work
    execute_on = 'TIMESTEP_END'
  []
[]

[Postprocessors]
  [L2u]
    type = ElementL2FunctorError
    approximate = vel_x
    exact = exact_u
  []
  [L2v]
    type = ElementL2FunctorError
    approximate = vel_y
    exact = exact_v
  []
  [L2phi]
    type = ElementL2FunctorError
    approximate = phase_2
    exact = exact_phi
  []
  [L2T]
    type = ElementL2FunctorError
    approximate = T_fluid
    exact = exact_T
  []
  # The energy equation carries grad(p) once the pressure work is active, so the pressure's own
  # accuracy stops being a private matter of the flow solve and becomes an input to the temperature.
  [L2p]
    type = ElementL2FunctorError
    approximate = pressure
    exact = exact_p
  []
  # The error in the assembled pressure work itself, against its manufactured value. This is the
  # quantity the energy equation consumes, and unlike L2p it is blind to the pressure datum, which
  # a segregated solve fixes only up to a constant.
  [L2pw]
    type = ElementL2FunctorError
    approximate = pw_sampled
    exact = mms_pressure_work
  []
[]

[Outputs]
  csv = true
[]
