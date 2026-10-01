# Verification of the algebraic slip closure and of the diffusion (drift) velocity derived from it,
# against their analytic values.
#
# Both manufactured solution studies of this model prescribe the slip velocity, so neither of them
# exercises the closure itself. This input does. The flow is not solved: the velocity field is held
# at zero, so the advective and transient contributions to the particle acceleration vanish and the
# acceleration is exactly gravity. Every remaining quantity is then known in closed form.
#
#   rho_m   = (1 - alpha) rho_c + alpha rho_d = 0.75 * 1000 + 0.25 * 200          = 800
#   tau_d   = rho_d dd^2 / (18 mu_c)          = 200 * 1e-4 / (18 * 1.0)           = 1.1111111111e-3
#   u_slip  = tau_d / f_drag * (rho_d - rho_m) / rho_d * g
#           = 1.1111111111e-3 / 1 * (200 - 800) / 200 * (-9.81)                   = 3.2700000000e-2
#   c_d     = alpha rho_d / rho_m             = 0.25 * 200 / 800                  = 0.0625
#   u_drift = (1 - c_d) u_slip                = 0.9375 * 3.2700000000e-2          = 3.0656250000e-2
#
# The relaxation time is formed from the CONTINUOUS phase viscosity, Fluent Theory Guide equation
# 16.4-13, not from the mixture viscosity of VTT equation (63). The mixture form there is a way of
# folding the swarm correction into the drag and is only valid with the apparent viscosity of Ishii
# and Zuber, VTT equation (43), which rises with the dispersed fraction; a volume averaged mixture
# viscosity falls with it. Concentration effects are carried by 'swarm_exponent' instead, which is
# zero here.
#
# The slip is positive while gravity points down because the dispersed phase is the lighter of the
# two, so it rises relative to the continuous phase.
#
# The second half of the input repeats the check with the Schiller and Naumann drag model, whose
# particle Reynolds number is formed from the slip velocity, Re_p = rho_c dd |u_slip| / mu_c, VTT
# equation (39). That makes the drag depend on the very quantity it determines, so the closure is
# the implicit scalar equation
#
#   s f(R s) = s0,   R = rho_c dd / mu_c = 1000 * 0.01 / 1 = 10,   s0 = 3.2700000000e-2
#
# with s0 the Stokes-limit slip computed above. Its root, obtained independently with a bracketing
# solver at machine tolerance, is
#
#   Re_p    = 3.0658697567e-1
#   f_drag  = 1.0665815118
#   u_slip  = 3.0658697567e-2      (note u_slip = Re_p / R exactly, by the definition of Re_p)
#   u_drift = 2.8742528969e-2      = 0.9375 * u_slip
#
# The last material takes the Ishii and Zuber multi-bubble relation, which is a correlation for the
# relative velocity itself rather than a drag law. With both frictional pressure gradients at their
# zero default it is, in the notation of Hibiki and Ishii (2003) equations (45) to (49),
#
#   v_inf   = sqrt(2) (g |rho_c - rho_d| sigma / rho_c^2)^(1/4)
#           = sqrt(2) (9.81 * 800 * 0.072 / 1e6)^(1/4)                         = 2.1804085208e-1
#   f       = (1 - alpha)^(3/2)                                                = 6.4951905284e-1
#   C       = 18.67 f / (1 + 17.67 f^(6/7))                                    = 9.1820435772e-1
#   u_slip  = v_inf sqrt(1 - alpha) C                                          = 1.7338353442e-1
#   u_drift = 0.9375 u_slip                                                    = 1.6254706352e-1
#
# The particle diameter does not enter, and neither does the swarm exponent: the correlation
# carries its own concentration dependence through f and C.
#
# References for the closure: Manninen, Taivassalo and Kallio, VTT Publications 288 (1996),
# equations (39), (58) and (63); ANSYS Fluent Theory Guide equations 16.4-12 to 16.4-15. For
# the slip to drift conversion, VTT equation (28).

# Named so that they cannot be shadowed: the slip material carries parameters called 'rho' and
# 'mu', and MOOSE resolves ${...} against the enclosing block before the global scope, so globals
# by those names would expand to the block's own values inside it.
rho_continuous = 1000.0
rho_dispersed = 200.0
mu_continuous = 1.0
mu_dispersed = 0.1
alpha = 0.25
dp = 0.01
dp_half = 0.005
sigma = 0.072
f_drag = 1.0
g = -9.81

[Mesh]
  [gen]
    type = GeneratedMeshGenerator
    dim = 2
    xmin = 0
    xmax = 1
    ymin = 0
    ymax = 1
    nx = 2
    ny = 2
  []
[]

[Problem]
  linear_sys_names = 'u_system v_system'
  # The closure is a pure function of the fields it is handed, so there is nothing to solve
  solve = false
[]

[Variables]
  [vel_x]
    type = MooseLinearVariableFVReal
    solver_sys = u_system
    initial_condition = 0
  []
  [vel_y]
    type = MooseLinearVariableFVReal
    solver_sys = v_system
    initial_condition = 0
  []
[]

[FunctorMaterials]
  [mixture]
    type = WCNSLinearFVMixtureFunctorMaterial
    phase_1_fraction = ${alpha}
    phase_1_names = '${rho_dispersed} ${mu_dispersed}'
    phase_2_names = '${rho_continuous}   ${mu_continuous}'
    prop_names = 'rho_mixture mu_mixture'
  []
  [slip_x]
    type = LinearWCNSFV2PSlipVelocityFunctorMaterial
    momentum_component = x
    u = vel_x
    v = vel_y
    rho = rho_mixture
    rho_d = ${rho_dispersed}
    mu = ${mu_continuous}
    fraction_dispersed = ${alpha}
    particle_diameter = ${dp}
    linear_coef_name = ${f_drag}
    gravity = '0 ${g} 0'
    slip_velocity_name = vel_slip_x
    drift_velocity_name = vel_drift_x
  []
  [slip_y]
    type = LinearWCNSFV2PSlipVelocityFunctorMaterial
    momentum_component = y
    u = vel_x
    v = vel_y
    rho = rho_mixture
    rho_d = ${rho_dispersed}
    mu = ${mu_continuous}
    fraction_dispersed = ${alpha}
    particle_diameter = ${dp}
    linear_coef_name = ${f_drag}
    gravity = '0 ${g} 0'
    slip_velocity_name = vel_slip_y
    drift_velocity_name = vel_drift_y
  []

  # The same closure with the drag correlation solved together with the force balance
  [slip_drag_x]
    type = LinearWCNSFV2PSlipVelocityFunctorMaterial
    momentum_component = x
    u = vel_x
    v = vel_y
    rho = rho_mixture
    rho_d = ${rho_dispersed}
    mu = ${mu_continuous}
    fraction_dispersed = ${alpha}
    particle_diameter = ${dp}
    use_dispersed_phase_drag_model = true
    rho_c = ${rho_continuous}
    gravity = '0 ${g} 0'
    slip_velocity_name = vel_slipdrag_x
    drift_velocity_name = vel_driftdrag_x
  []
  [slip_drag_y]
    type = LinearWCNSFV2PSlipVelocityFunctorMaterial
    momentum_component = y
    u = vel_x
    v = vel_y
    rho = rho_mixture
    rho_d = ${rho_dispersed}
    mu = ${mu_continuous}
    fraction_dispersed = ${alpha}
    particle_diameter = ${dp}
    use_dispersed_phase_drag_model = true
    rho_c = ${rho_continuous}
    gravity = '0 ${g} 0'
    slip_velocity_name = vel_slipdrag_y
    drift_velocity_name = vel_driftdrag_y
  []

  # The deformed bubble law. Its terminal velocity is independent of the particle size, so the same
  # material is instantiated twice at diameters a factor of two apart and must return the same
  # speed; that is the property the closure is being checked for, not merely a value.
  [slip_dist_y]
    type = LinearWCNSFV2PSlipVelocityFunctorMaterial
    momentum_component = y
    u = vel_x
    v = vel_y
    rho = rho_mixture
    rho_d = ${rho_dispersed}
    mu = ${mu_continuous}
    fraction_dispersed = ${alpha}
    particle_diameter = ${dp}
    use_dispersed_phase_drag_model = true
    drag_model = 'distorted-particle'
    surface_tension = ${sigma}
    rho_c = ${rho_continuous}
    gravity = '0 ${g} 0'
    slip_velocity_name = vel_slipdist_y
    drift_velocity_name = vel_driftdist_y
  []
  [slip_dist_half_y]
    type = LinearWCNSFV2PSlipVelocityFunctorMaterial
    momentum_component = y
    u = vel_x
    v = vel_y
    rho = rho_mixture
    rho_d = ${rho_dispersed}
    mu = ${mu_continuous}
    fraction_dispersed = ${alpha}
    particle_diameter = ${dp_half}
    use_dispersed_phase_drag_model = true
    drag_model = 'distorted-particle'
    surface_tension = ${sigma}
    rho_c = ${rho_continuous}
    gravity = '0 ${g} 0'
    slip_velocity_name = vel_slipdisthalf_y
    drift_velocity_name = vel_driftdisthalf_y
  []

  # The automatic branch takes whichever of the two single particle laws resists more, so at these
  # conditions it must return the rigid sphere result and not the deformed one. The two differ by a
  # factor of six here, so a wrong selection cannot pass.
  [slip_auto_y]
    type = LinearWCNSFV2PSlipVelocityFunctorMaterial
    momentum_component = y
    u = vel_x
    v = vel_y
    rho = rho_mixture
    rho_d = ${rho_dispersed}
    mu = ${mu_continuous}
    fraction_dispersed = ${alpha}
    particle_diameter = ${dp}
    use_dispersed_phase_drag_model = true
    drag_model = 'automatic'
    surface_tension = ${sigma}
    rho_c = ${rho_continuous}
    gravity = '0 ${g} 0'
    slip_velocity_name = vel_slipauto_y
    drift_velocity_name = vel_driftauto_y
  []

  # The Ishii and Zuber multi-bubble relation, see the header for its closed form here
  [slip_iz_y]
    type = LinearWCNSFV2PSlipVelocityFunctorMaterial
    momentum_component = y
    u = vel_x
    v = vel_y
    rho = rho_mixture
    rho_d = ${rho_dispersed}
    mu = ${mu_continuous}
    fraction_dispersed = ${alpha}
    particle_diameter = ${dp}
    use_dispersed_phase_drag_model = true
    drag_model = 'ishii-zuber'
    surface_tension = ${sigma}
    rho_c = ${rho_continuous}
    gravity = '0 ${g} 0'
    slip_velocity_name = vel_slipiz_y
    drift_velocity_name = vel_driftiz_y
  []

  # The drag function evaluated from the resulting slip velocity. This closes the loop: it must
  # reproduce the f_drag that the closure above solved for, and it demonstrates that a drag
  # material built the correct way does not recurse back into the slip velocity.
  [drag]
    type = NSFVDispersePhaseDragFunctorMaterial
    drag_coef_name = drag_function
    u = vel_slipdrag_x
    v = vel_slipdrag_y
    rho = ${rho_continuous}
    mu = ${mu_continuous}
    particle_diameter = ${dp}
  []
[]

[Postprocessors]
  [rho_mixture]
    type = ElementAverageFunctorPostprocessor
    functor = rho_mixture
  []
  [mu_mixture]
    type = ElementAverageFunctorPostprocessor
    functor = mu_mixture
  []
  # Gravity acts along y, so the x components must be identically zero
  [slip_x]
    type = ElementAverageFunctorPostprocessor
    functor = vel_slip_x
  []
  [slip_y]
    type = ElementAverageFunctorPostprocessor
    functor = vel_slip_y
  []
  [drift_y]
    type = ElementAverageFunctorPostprocessor
    functor = vel_drift_y
  []
  # The ratio of the two is the mass fraction complement, 1 - c_d, independently of the closure
  [drift_over_slip_y]
    type = ParsedPostprocessor
    expression = 'drift_y / slip_y'
    pp_names = 'drift_y slip_y'
  []

  # With the drag correlation solved self-consistently
  [slipdrag_y]
    type = ElementAverageFunctorPostprocessor
    functor = vel_slipdrag_y
  []
  [driftdrag_y]
    type = ElementAverageFunctorPostprocessor
    functor = vel_driftdrag_y
  []
  [drag_function]
    type = ElementAverageFunctorPostprocessor
    functor = drag_function
  []
  # Re_p = R |u_slip| with R = 10, so this must equal the drag function evaluated at that Re_p.
  # It is the consistency of the implicit solve, not a second independent quantity.
  # The deformed bubble branch, and the size independence of its terminal velocity: the two speeds
  # below must agree exactly, the particle diameter cancelling between the relaxation time and the
  # drag coefficient.
  [slipdist_y]
    type = ElementAverageFunctorPostprocessor
    functor = vel_slipdist_y
  []
  [slipdist_half_y]
    type = ElementAverageFunctorPostprocessor
    functor = vel_slipdisthalf_y
  []
  [dist_size_ratio]
    type = ParsedPostprocessor
    expression = 'slipdist_half_y / slipdist_y'
    pp_names = 'slipdist_half_y slipdist_y'
  []
  # The automatic branch, which must return the rigid sphere speed here
  [slipauto_y]
    type = ElementAverageFunctorPostprocessor
    functor = vel_slipauto_y
  []
  [auto_over_drag]
    type = ParsedPostprocessor
    expression = 'slipauto_y / slipdrag_y'
    pp_names = 'slipauto_y slipdrag_y'
  []

  # The Ishii and Zuber relation, against the closed form in the header
  [slipiz_y]
    type = ElementAverageFunctorPostprocessor
    functor = vel_slipiz_y
  []
  [driftiz_y]
    type = ElementAverageFunctorPostprocessor
    functor = vel_driftiz_y
  []

  [slip_times_drag]
    type = ParsedPostprocessor
    expression = '-slipdrag_y * drag_function'
    pp_names = 'slipdrag_y drag_function'
  []
[]

[Executioner]
  type = Steady
[]

[Outputs]
  csv = true
[]
